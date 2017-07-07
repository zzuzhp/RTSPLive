#include "RTSPCore.h"
#include "XUtil/XULog.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////

RTSPCore::RTSPCore() : m_ute(nullptr),
                       m_messager(nullptr),
                       m_sdpenc(nullptr),
                       m_clientId(0),
                       m_streamId(1),
                       m_pt(96),         ///< RFC3551: 96-127  dynamic
                       m_portbase(57092) ///< a random large port number to reduce the probability of bind failure
{

}

RTSPCore::~RTSPCore()
{
    tear();
}

bool 
RTSPCore::build(uint16_t listen_port)
{
    {
        XUMutexGuard mon(m_lock);

        m_ute = UTECreate();
        if (!m_ute)
        {
            goto fail;
        }

        m_sdpenc = new SDPEncoder();
        if (!m_sdpenc)
        {
            goto fail;
        }

        m_messager = new RTSPMessager(this, m_ute, listen_port);
        if (!m_messager)
        {
            goto fail;
        }

        create_logger<XULoggerSystem>();
        create_logger<XULoggerConsole>();

        return true;
    }

fail:
    
    tear();

    return false;
}

void 
RTSPCore::tear()
{
    XUMutexGuard mon(m_lock);

    stop();

    if (m_ute)
    {
        UTEDestroy(m_ute);
        m_ute = nullptr;
    }

    if (m_messager)
    {
        delete m_messager;
        m_messager = nullptr;
    }

    if (m_sdpenc)
    {
        delete m_sdpenc;
        m_sdpenc = nullptr;
    }
}

bool 
RTSPCore::start()
{
    XUMutexGuard mon(m_lock);

    if (!m_messager || !m_ute)
    {
        return false;
    }

    if (!m_ute->start())
    {
        return false;
    }

    return true;
}

void 
RTSPCore::stop()
{
    XUMutexGuard mon(m_lock);
    m_ute->stop();
}

uint32_t 
RTSPCore::add_stream(RTSP_MEDIA type, void *init_param)
{
    XUMutexGuard mon(m_lock);

    IAVStream * stream = create_stream(m_streamId++, type, init_param);
    if (stream->avp_type() == -1)
    {
        stream->set_avp_type(m_pt++);
    }

    m_sdpenc->add_media(stream, stream->avp_type());
    m_streams.push_back(stream);

    return stream->stream_id();
}

void 
RTSPCore::push_stream(uint32_t streamId, RTSP_media_frame *frame)
{
    XUMutexGuard mon(m_lock);

    IAVStream * stream = nullptr;
    std::vector<IAVStream *>::iterator itr = m_streams.begin();
    for (; itr != m_streams.end(); ++itr)
    {
        if ((*itr)->stream_id() == streamId)
        {
            stream = *itr;
            break;
        }
    }

    if (!stream)
    {
        return;
    }

    stream->push_frame(frame->frame, frame->len, frame->ts_ms);
}

RTSPClient * 
RTSPCore::get_client(uint32_t id)
{
    std::vector<RTSPClient *>::iterator itr = m_clients.begin();
    for (; itr != m_clients.end(); ++itr)
    {
        if ((*itr)->id() == id)
        {
            return *itr;
        }
    }

    return nullptr;
}

void 
RTSPCore::remove_client(uint32_t id)
{
    std::vector<RTSPClient *>::iterator itr = m_clients.begin();
    for (; itr != m_clients.end(); ++itr)
    {
        if ((*itr)->id() == id)
        {
            m_clients.erase(itr);
            break;
        }
    }
}

uint32_t 
RTSPCore::on_new_client(std::string ip, uint16_t port)
{
    /* we accept all clients */
    XULOG_D("new client from: %s:%d", ip.c_str(), port);
    RTSPClient * client = new RTSPClient(this, ++m_clientId, m_ute, ip);
    if (!client)
    {
        return 0;
    }

    m_clients.push_back(client);

    return client->id();
}

void 
RTSPCore::on_rtsp_options(uint32_t clientId, RTSPRequest *request)
{
    XULOG_D("message: OPTIONS from client: %d.", clientId);
    m_messager->send_response(clientId, request, "200", "OK", nullptr);
}

void
RTSPCore::on_rtsp_describe(uint32_t clientId, RTSPRequest *request)
{
    XULOG_D("message: DESCRIBE from client: %d.", clientId);
    m_messager->send_response(clientId, request, "200", "OK", (void *)(m_sdpenc->sdp().c_str()));
}

void
RTSPCore::on_rtsp_setup(uint32_t clientId, RTSPRequest *request, RTSP_Setup *setup)
{
    XULOG_D("message: SETUP from client: %d.", clientId);

    IAVStream  * stream = nullptr;
    RTSPClient * client = get_client(clientId);
    if (!client)
    {
        XULOG_E("no such a client: %d", clientId);
        return;
    }

    std::vector<IAVStream *>::iterator itr = m_streams.begin();
    for (; itr != m_streams.end(); ++itr)
    {
        if ((*itr)->stream_id() == setup->stream)
        {
            stream = *itr;
            break;
        }
    }

    if (!stream)
    {
        XULOG_E("no such a stream: %d", setup->stream);
        m_messager->send_response(clientId, request, "451", "Invalid URL target", nullptr);
    }
    else
    {
        bool ret = false;
        if (setup->tcp)
        {
            /* Certain firewall designs and other circumstances may force a server
               to interleave RTSP methods and stream data. Interleaved binary
               data SHOULD only be used if RTSP is carried over TCP.*/
            ret = client->add_stream(stream, m_messager->rtsp_transport(clientId), setup->interleaved);
        }
        else
        {
            setup->s_port_base = get_rtp_port(); //TODO: port wraps
            ret = client->add_stream(stream, setup->port_base, "", setup->s_port_base);
        }

        if (ret)
        {
            /*TODO: check for socket bind failure due to port unavailable(occupied) */
            setup->session = client->session();
            stream->add_observer(client);
            m_messager->send_response(clientId, request, "200", "OK", setup);
        }
        else
        {
            m_messager->send_response(clientId, request, "503", "Service Unavailable", nullptr);
        }
    }
}

void
RTSPCore::on_rtsp_play(uint32_t clientId, RTSPRequest *request, RTSP_Play *play)
{
    XULOG_D("message: PLAY from client: %d.", clientId);

    RTSPClient * client = get_client(clientId);
    if (!client)
    {
        XULOG_E("no such a client: %d", clientId);
        m_messager->send_response(clientId, request, "451", "Invalid URL target", play);
        return;
    }

    /* first, send the play ok response */
    m_messager->send_response(clientId, request, "200", "OK", play);

    /* and then, send the media data */
    std::vector<IAVStream *>::iterator itr = m_streams.begin();
    for (; itr != m_streams.end(); ++itr)
    {
        if (play->stream == -1 || (*itr)->stream_id() == play->stream)
        {
            client->active_stream(*itr);
        }
    }
}

void
RTSPCore::on_rtsp_teardown(uint32_t clientId, RTSPRequest *request, RTSP_Teardown *tear)
{
    XULOG_D("message: TEARDOWN from client: %d.", clientId);

    RTSPClient * client = get_client(clientId);
    if (!client)
    {
        XULOG_E("no such a client: %d", clientId);
        m_messager->send_response(clientId, request, "451", "Invalid URL target", tear);
        return;
    }

    std::vector<IAVStream *>::iterator itr = m_streams.begin();
    for (; itr != m_streams.end(); ++itr)
    {
        if (tear->stream == -1 || (*itr)->stream_id() == tear->stream)
        {
            client->del_stream(*itr);
            (*itr)->remove_observer(client);
        }
    }
#if 0
    if (client->stream_size() == 0)
    {
        /* no streams: del this client */
        delete client;
        remove_client(clientId);
    }
#endif
    m_messager->send_response(clientId, request, "200", "OK", tear);
}

void
RTSPCore::on_rtcp(uint32_t id, RtcpPacket *rtcp)
{

}

void
RTSPCore::on_time_out(RTSPClient * client)
{
    XULOG_D("client timed out!");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////
