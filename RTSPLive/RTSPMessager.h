#ifndef ___RTSPMESSAGER_H___
#define ___RTSPMESSAGER_H___

#include "UTE/UTE.h"
#include "XUtil/XULog.h"

#include <string>
#include <map>
#include <vector>
#include <memory>

class RtcpPacket;
////////////////////////////////////////////////////////////////////////////////////////////////////
//// decode/encode a RTSP(RFC2326) message

/* format of a request message:
    Method URL Ver CRLF
    Field1: value CRLF
       .
       .
       .
    Fieldn: value CRLF
    CRLF
    Body(often emitted)
 */

/* format of a response message:
   Ver status Phrase CRLF
   Field1: value CRLF
      .
      .
      .
   Fieldn: value CRLF
   CRLF
   Body
 */
struct RTSPMethod
{
    enum METHOD
    {
        RTSP_METHOD_OPTIONS  = 0,
        RTSP_METHOD_DESCRIBE = 1,
        RTSP_METHOD_SETUP    = 2,
        RTSP_METHOD_PLAY     = 3,
        RTSP_METHOD_TEARDOWN = 4
    } method;

    std::string name;
};

struct RTSP_Setup
{
    bool        tcp;            ///< tcp or udp
    bool        unicast;        ///< unicast or multicast
    uint16_t    port_base;      ///< UDP: client_port: 
    uint8_t     interleaved[2]; ///< TCP: interleaved
    int         stream;         ///< stream id : -1 if absent

    /* response */
    std::string session;
    uint16_t    s_port_base;    ///< server_port:
};

struct RTSP_Play
{
    int         stream;         ///< stream id : -1 if absent
};

struct RTSP_Teardown
{
    int         stream;         ///< stream id : -1 if absent
};

typedef std::map<std::string, std::string> RTSPField;
typedef std::map<uint32_t, std::shared_ptr<IUTETransport>> RTSPMessagerClient;

struct RTSPRequest
{
    RTSPMethod      method;
    std::string     url;
    std::string     version;
    RTSPField       fields;
};

struct RTSPResponse
{
    std::string     status;
    std::string     phrase;
    std::string     response;
};

class IRTSPMessagerSink
{
public:

    virtual uint32_t on_new_client(std::string ip, uint16_t port) = 0;

    virtual void on_rtsp_options(uint32_t id, RTSPRequest * request) = 0;

    virtual void on_rtsp_describe(uint32_t id, RTSPRequest * request) = 0;

    virtual void on_rtsp_setup(uint32_t id, RTSPRequest * request, RTSP_Setup * param) = 0;

    virtual void on_rtsp_play(uint32_t id, RTSPRequest * request, RTSP_Play * param) = 0;

    virtual void on_rtsp_teardown(uint32_t id, RTSPRequest * request, RTSP_Teardown * param) = 0;

    virtual void on_rtcp(uint32_t id, RtcpPacket *rtcp) = 0;
};

class RTSPMessager : public IUTEAcceptorObserver,
                     public IUTETransportObserver
{
public:

    RTSPMessager(IRTSPMessagerSink * sink, IUTE * ute, uint16_t port);

    ~RTSPMessager();

    void send_response(uint32_t id, RTSPRequest * request, std::string status, std::string phrase, void * arg);

    std::shared_ptr<IUTETransport> rtsp_transport(uint32_t id);

private:

    bool create_request(std::string str, RTSPRequest ** request);

    bool parse_header(std::string line, RTSPRequest * request);

    bool parse_transport(std::string line, RTSP_Setup * param);

    void fill_response(RTSPRequest * request, RTSPResponse * response, void * arg);

    std::string get_field(RTSPRequest * request, std::string field);

    std::string trim(std::string value);

    bool check_request(RTSPRequest * request);

    uint32_t get_client_id(std::shared_ptr<IUTETransport> transport);

    int get_stream_id(std::string url);

    /* UTE callback */
    void on_accept(std::shared_ptr<IUTEAcceptor> acceptor, std::shared_ptr<IUTETransport> transport);

    void on_recv(std::shared_ptr<IUTETransport> transport, const char * data, int len);

    void on_send(std::shared_ptr<IUTETransport> transport, int len) { /* XULOG_D("RTSPMessager send %d bytes.", len); */ }

    void on_message(std::shared_ptr<IUTETransport> transport, int code, const char * message, void * arg) { /* XULOG_D("RTSPMessager message: %s", message); */ }

private:

    IRTSPMessagerSink               * m_sink;
    std::shared_ptr<IUTEAcceptor>     m_acceptor;
    RTSPMessagerClient                m_clients;
    std::vector<RTSPMethod>           m_methods;
};

/*   Code  Phase
    ¡±100¡±; Continue(all 100 range)
    ¡°110¡±; Connect Timeout
    ¡°200¡±; OK
    ¡±201¡±; Created
    ¡±250¡±; Low on Storage Space
    ¡±300¡±; Multiple Choices
    ¡±301¡±; Moved Permanently
    ¡±302¡±; Moved Temporarily
    ¡±303¡±; See Other
    ¡±304¡±; Not Modified
    ¡±305¡±; Use Proxy
    ¡±350¡±; Going Away
    ¡±351¡±; Load Balancing
    ¡±400¡±; Bad Request
    ¡±401¡±; Unauthorized
    ¡±402¡±; Payment Required
    ¡±403¡±; Forbidden
    ¡±404¡±; Not Found
    ¡±405¡±; Method Not Allowed
    ¡±406¡±; Not Acceptable
    ¡±407¡±; Proxy Authentication Required
    ¡±408¡±; Request Time - out
    ¡±410¡±; Gone
    ¡±411¡±; Length Required
    ¡±412¡±; Precondition Failed
    ¡±413¡±; Request Entity Too Large
    ¡±414¡±; Request - URI Too Large
    ¡±415¡±; Unsupported Media Type
    ¡±451¡±; Parameter Not Understood
    ¡±452¡±; reserved
    ¡±453¡±; Not Enough Bandwidth
    ¡±454¡±; Session Not Found
    ¡±455¡±; Method Not Valid in This State
    ¡±456¡±; Header Field Not Valid for Resource
    ¡±457¡±; Invalid Range
    ¡±458¡±; Parameter Is Read - Only
    ¡±459¡±; Aggregate operation not allowed
    ¡±460¡±; Only aggregate operation allowed
    ¡±461¡±; Unsupported transport
    ¡±462¡±; Destination unreachable
    ¡±500¡±; Internal Server Error
    ¡±501¡±; Not Implemented
    ¡±502¡±; Bad Gateway
    ¡±503¡±; Service Unavailable
    ¡±504¡±; Gateway Time - out
    ¡±505¡±; RTSP Version not supported
    ¡±551¡±; Option not supported 
 */

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___RTSPMESSAGER_H___
