#include "RTSPMessager.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#define RTSP_ENDLINE            "\r\n"
#define RTSP_SEPERATOR          " "         ///< whitespace
#define RTSP_FIELD_SEP          ":"
#define RTSP_ATTR_SEP           ";"
#define RTSP_VERSION            "RTSP/1.0"
#define RTSP_CSEQ               "CSeq"
#define RTSP_ACCEPT             "Accept"
#define RTSP_ACCEPT_VAL         "application/sdp"
#define RTSP_TRANSPORT          "Transport"
#define RTSP_TRANS_UDP          "RTP/AVP"
#define RTSP_TRANS_TCP          "RTP/AVP/TCP"
#define RTSP_TRANS_UNICAST      "unicast"
#define RTSP_TRANS_MULTICAST    "multicast"
#define RTSP_CLIENT_PORT        "client_port="
#define RTSP_SERVER_PORT        "server_port="
#define RTSP_FIELD(x, r)        (std::string(x) + std::string(": ") + std::string(get_field(r, x)))
#define RTSPMETHOD(m)           {RTSPMethod::METHOD::RTSP_METHOD_##m, #m}

RTSPMessager::RTSPMessager(IRTSPMessagerSink * sink, IUTE * ute, uint16_t port) : m_sink(sink)
{
    /* all supported methods */
    m_methods.push_back(RTSPMETHOD(OPTIONS));
    m_methods.push_back(RTSPMETHOD(DESCRIBE));
    m_methods.push_back(RTSPMETHOD(SETUP));
    m_methods.push_back(RTSPMETHOD(PLAY));
    m_methods.push_back(RTSPMETHOD(TEARDOWN));

    if (!ute || !(m_acceptor = ute->create_acceptor(this)))
    {
        XULOG_E("ute not exist or create_acceptor in rtsp messager.");
    }

    if (m_acceptor)
    {
        m_acceptor->accept(port);
    }
}

RTSPMessager::~RTSPMessager()
{
    m_acceptor.reset();

    while (!m_clients.empty())
    {
        std::map<uint32_t, std::shared_ptr<IUTETransport>>::iterator itr = m_clients.begin();
        m_clients.erase(itr);

        itr->second.reset();
    }
}

bool 
RTSPMessager::create_request(std::string str, RTSPRequest ** request)
{
    std::string::size_type   pos        = 0;
    std::string::size_type   start_pos  = 0;
    std::vector<std::string> lines;

    if (!request)
    {
        return false;
    }

    *request = new RTSPRequest;

    while ((pos = str.find(RTSP_ENDLINE, start_pos)) != std::string::npos)
    {
        std::string line = str.substr(start_pos, pos - start_pos);
        
        if (line.length())
        {
            lines.push_back(line);
        }

        start_pos = pos + std::string(RTSP_ENDLINE).length();
    }

    /* parse message request-line */
    std::vector<std::string>::iterator itr = lines.begin();
    for (; itr != lines.end(); ++itr)
    {
        std::string line = *itr;
        if (itr == lines.begin())
        {
            parse_header(line, *request);
        }
        else if ((pos = line.find(RTSP_FIELD_SEP)) != std::string::npos)
        {
            std::string field = line.substr(0, pos);
            std::string value = trim(line.substr(pos + 1));

            (*request)->fields.insert(std::pair<std::string, std::string>(field, value));
        }
    }

    return true;
}

void
RTSPMessager::send_response(uint32_t      id, 
                            RTSPRequest * request, 
                            std::string   status, 
                            std::string   phrase, 
                            void        * arg)
{
    RTSPResponse response;
    /* we have an unsupported method? */
    if (m_clients.find(id) == m_clients.end())
    {
        XULOG_E("no such a trans id, invalid param.");
        return;
    }

    if (!check_request(request))
    {
        XULOG_E("send response failed, invalid param.");
        return;
    }

    /* format a response */
    response.status = status;
    response.phrase = phrase;
    
    response.response += RTSP_VERSION;
    response.response += RTSP_SEPERATOR;
    response.response += status;
    response.response += RTSP_SEPERATOR;
    response.response += phrase;
    response.response += RTSP_ENDLINE;
    response.response += RTSP_FIELD(RTSP_CSEQ, request);
    response.response += RTSP_ENDLINE;

    /* fill in the fields */
    fill_response(request, &response, arg);

    m_clients[id]->send((char *)response.response.c_str(), response.response.length());
    XULOG_D("send response: \n%s", response.response.c_str());
}

void
RTSPMessager::parse_header(std::string line, RTSPRequest * request)
{
    std::string::size_type p, pos;
    std::string method;

    /* method */
    pos     = line.find(RTSP_SEPERATOR, 0);
    method  = line.substr(0, pos != std::string::npos ? pos - 0 : std::string::npos);

    for (std::vector<RTSPMethod>::iterator itr = m_methods.begin(); itr != m_methods.end(); ++itr)
    {
        if (itr->name.compare(method) == 0)
        {
            request->method = *itr;
            break;
        }
    }

    /* url */
    p   = pos + std::string(RTSP_SEPERATOR).length();
    pos = line.find(RTSP_SEPERATOR, p);
    request->url = line.substr(p, pos != std::string::npos ? pos - p : std::string::npos);

    /* version */
    p   = pos + std::string(RTSP_SEPERATOR).length();
    pos = line.find(RTSP_SEPERATOR, p);
    request->version = line.substr(p, pos != std::string::npos ? pos - p : std::string::npos);
}

bool 
RTSPMessager::parse_transport(std::string line, RTSP_Setup * param)
{
    std::string attr;
    std::string::size_type pos, pos2, idx;

    /* transport specifier (RTP/AVP or RTP/AVP/TCP) */
    if (std::string::npos != line.find(RTSP_TRANS_TCP))
    {
        param->tcp = true;
    }
    else if (std::string::npos != line.find(RTSP_TRANS_UDP))
    {
        param->tcp = false;
    }
    else
    {
        XULOG_E("no transport specifier in SETUP request.");
        return false;
    }

    /* unicast or multicast */
    if (std::string::npos != line.find(RTSP_TRANS_UNICAST))
    {
        param->unicast = true;
    }
    else if (std::string::npos != line.find(RTSP_TRANS_MULTICAST))
    {
        param->unicast = false;
    }
    else
    {
        XULOG_E("unicast/multicast not found in SETUP request.");
        return false;
    }

    /* client_port=xxx-xxx; */
    pos = line.find(RTSP_CLIENT_PORT);
    if (pos == std::string::npos)
    {
        XULOG_E("no client port in SETUP request.");
        return false;
    }

    pos += std::string(RTSP_CLIENT_PORT).length();
    pos2 = line.find(RTSP_ATTR_SEP, pos);

    attr = line.substr(pos, pos2 != std::string::npos ? pos2 - pos : std::string::npos);

    try
    {
        param->port_base = std::stoi(attr, &idx);
        
        int rtcp_port = std::stoi(attr.substr(idx + 1));
        if (rtcp_port != param->port_base + 1)
        {
            XULOG_E("invalid client ports -- rtcp.");
            return false;
        }
    }
    catch (const std::invalid_argument &)
    {
        XULOG_E("invalid client ports.");
        return false;
    }

    param->s_port_base = 0;

    return true;
}

std::string
RTSPMessager::get_field(RTSPRequest * request, std::string field)
{
    RTSPField::iterator itr = request->fields.find(field);
    if (itr != request->fields.end())
    {
        return itr->second;
    }

    return std::string();
}

std::string
RTSPMessager::trim(std::string value)
{
    std::string::size_type p0 = value.find_first_not_of(RTSP_SEPERATOR);
    if (p0 == std::string::npos)
    {
        return "";
    }

    std::string::size_type p1 = value.find_last_not_of(RTSP_SEPERATOR);

    return value.substr(p0, p1 - p0 + 1);
}

void
RTSPMessager::fill_response(RTSPRequest * request, RTSPResponse * response, void * arg)
{
    if (request->method.method == RTSPMethod::METHOD::RTSP_METHOD_OPTIONS)
    {
        /* 10.1 An OPTIONS request may be issued at any time */
        response->response += "Public: ";

        std::vector<RTSPMethod>::iterator itr = m_methods.begin();
        for (; itr != m_methods.end();)
        {
            response->response += itr->name;
            if (++itr != m_methods.end())
            {
                response->response += ", ";
            }
        }

        response->response += RTSP_ENDLINE;

        response->response += RTSP_ENDLINE;
    }
    else if (request->method.method == RTSPMethod::METHOD::RTSP_METHOD_DESCRIBE)
    {
        std::string body((const char *)arg);

        response->response += std::string("Content-Type: application/sdp");
        response->response += RTSP_ENDLINE;
        response->response += (std::string("Content-Length: ") + std::to_string(body.length()));
        response->response += RTSP_ENDLINE;

        response->response += RTSP_ENDLINE;

        /* append the sdp */
        response->response += body; ///< body
    }
    else if (request->method.method == RTSPMethod::METHOD::RTSP_METHOD_SETUP)
    {
        /* TODO: 
           A client can issue a SETUP request for a stream that is already playing to 
           change transport parameters, which a server MAY allow. If it does not allow 
           this, it MUST respond with error "455 Method Not Valid In This State". 
         */
        RTSP_Setup * setup = (RTSP_Setup *)arg;
        if (!setup)
        {
            /* error in setup resquest(e.g. url not valid) */
            return;
        }

        /* The server generates session identifiers in response to SETUP requests.
           The session identifier is needed to distinguish several delivery
           requests for the same URL coming from the same client. (zhp: don't know what it means, but it seems everything goes fine without it.)
         */
        response->response += std::string("Session: ") + setup->session;
        response->response += std::string(";timeout=") + std::to_string(60); ///< 60s timeout
        response->response += RTSP_ENDLINE;

        /* the response will contain the transport parameters selected by the server */
        response->response += std::string("Transport: ") + (setup->tcp ? RTSP_TRANS_TCP : RTSP_TRANS_UDP);
        response->response += RTSP_ATTR_SEP;
        response->response += (setup->unicast ? RTSP_TRANS_UNICAST : RTSP_TRANS_MULTICAST);
        response->response += RTSP_ATTR_SEP;
        response->response += RTSP_CLIENT_PORT + std::to_string(setup->port_base) + "-" + std::to_string(setup->port_base + 1);
        response->response += RTSP_ATTR_SEP;

        /* plus the server port */
        response->response += RTSP_SERVER_PORT + std::to_string(setup->s_port_base) + "-" + std::to_string(setup->s_port_base + 1);
        response->response += RTSP_ENDLINE;
        
        response->response += RTSP_ENDLINE;
    }
    else if (request->method.method == RTSPMethod::METHOD::RTSP_METHOD_PLAY)
    {
        response->response += RTSP_ENDLINE;
    }
    else if (request->method.method == RTSPMethod::METHOD::RTSP_METHOD_TEARDOWN)
    {
        response->response += RTSP_ENDLINE;
    }
}

bool
RTSPMessager::check_request(RTSPRequest * request)
{
    if (!request)
    {
        return false;
    }

    std::vector<RTSPMethod>::iterator itr = m_methods.begin();
    for (; itr != m_methods.end(); ++itr)
    {
        if (itr->method == request->method.method)
        {
            break;
        }
    }

    if (itr == m_methods.end())
    {
        XULOG_E("unknown method: %s", request->method);
        return false;
    }

    /* 12.17 CSeq
      The CSeq field specifies the sequence number for an RTSP request-response pair.
      This field MUST be present in all requests and responses.
    */
    std::string cseq = get_field(request, RTSP_CSEQ);
    if (cseq.length() == 0)
    {
        XULOG_E("CSeq field not found!");
        return false;
    }

    /* for describe: we only support 'application/sdp' */
    if (request->method.method == RTSPMethod::METHOD::RTSP_METHOD_DESCRIBE)
    {
        std::string accept = get_field(request, RTSP_ACCEPT);
        if (std::string::npos == accept.find(RTSP_ACCEPT_VAL, 0))
        {
            XULOG_E("invalid accept field: %s", accept.empty() ? "(null)" : accept.c_str());
            return false;
        }
    }

    return true;
}

uint32_t
RTSPMessager::get_client(std::shared_ptr<IUTETransport> transport)
{
    std::map<uint32_t, std::shared_ptr<IUTETransport>>::iterator itr = m_clients.begin();
    for (; itr != m_clients.end(); ++itr)
    {
        if (itr->second == transport)
        {
            return itr->first;
        }
    }

    return 0;
}

int 
RTSPMessager::get_stream_id(std::string url)
{
    std::string tag_stream_id("streamid=");
    std::string s_stream_id = url.substr(url.find_last_of("/") + 1);
    if ((s_stream_id.length() == 0) ||
        s_stream_id.compare(0, tag_stream_id.length(), tag_stream_id) != 0)
    {
        return -1; ///< invalid
    }
    else
    {
        return std::stoi(s_stream_id.substr(tag_stream_id.length()));
    }
}

void 
RTSPMessager::on_accept(std::shared_ptr<IUTEAcceptor> acceptor, std::shared_ptr<IUTETransport> transport)
{
    if (acceptor != m_acceptor)
    {
        XULOG_E("unknown acceptor");
        return;
    }

    uint32_t client_id = m_sink->on_new_client(transport->remote_ip(), transport->remote_port());
    if (client_id != 0)
    {
        transport->set_observer(this); ///< this will start read
        m_clients[client_id] = transport;
    }
}

void
RTSPMessager::on_recv(std::shared_ptr<IUTETransport> transport, char * data, int len)
{
    /* RTSP messages from clients */
    uint32_t clientId = get_client(transport);
    if (!clientId)
    {
        XULOG_E("unknown message from %s:%d", transport->remote_ip(), transport->remote_port());
        return;
    }

    RTSPRequest * request = NULL;
    if (!create_request(std::string(data, len), &request))
    {
        XULOG_E("create request failed for string: %s", data);
        return;
    }
    else
    {
        XULOG_D("\nmethod : %s\nurl    : %s\nversion: %s", request->method.name.c_str(), request->url.c_str(), request->version.c_str());
    }

    RTSPField::iterator itr = request->fields.begin();
    for (; itr != request->fields.end(); ++itr)
    {
        XULOG_D("field  : %s: %s", itr->first.c_str(), itr->second.c_str());
    }

    do
    {
        if (request->method.method == RTSPMethod::METHOD::RTSP_METHOD_OPTIONS)
        {
            m_sink->on_rtsp_options(clientId, request);
        }
        else if (request->method.method == RTSPMethod::METHOD::RTSP_METHOD_DESCRIBE)
        {
            m_sink->on_rtsp_describe(clientId, request);
        }
        else if (request->method.method == RTSPMethod::METHOD::RTSP_METHOD_SETUP)
        {
            RTSP_Setup setup;
            if (!parse_transport(get_field(request, RTSP_TRANSPORT), &setup))
            {
                break;
            }

            setup.stream = get_stream_id(request->url);

            m_sink->on_rtsp_setup(clientId, request, &setup);
        }
        else if (request->method.method == RTSPMethod::METHOD::RTSP_METHOD_PLAY)
        {
            RTSP_Play play;
            play.stream = get_stream_id(request->url);

            m_sink->on_rtsp_play(clientId, request, &play);
        }
        else if (request->method.method == RTSPMethod::METHOD::RTSP_METHOD_TEARDOWN)
        {
            RTSP_Teardown tear;
            tear.stream = get_stream_id(request->url);

            m_sink->on_rtsp_teardown(clientId, request, &tear);
        }
    } while (0);

    delete request;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////
