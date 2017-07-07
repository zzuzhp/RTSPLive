#ifndef ___RTSPCLIENT_H___
#define ___RTSPCLIENT_H___

#include "AVStream.h"
#include "UTE/UTE.h"
#include "XUtil/XUMutex.h"
#include "XUtil/XULog.h"

#include <list>

////////////////////////////////////////////////////////////////////////////////////////////////////
////

class RTSPClient;
class IRTSPClientSink
{
public:
    virtual void on_time_out(RTSPClient * client) = 0;
};

struct NetStream
{
    bool                             active;
    bool                             sync;
    uint8_t                          channelid[2];
    char                           * interpkt;
    IAVStream                      * stream;
    std::shared_ptr<IUTETransport>   transport;
};

class RTSPClient : public IAVStreamObserver,
                   public IUTETransportObserver
{
public:

    RTSPClient(IRTSPClientSink * sink,
               uint32_t          id,
               IUTE            * ute,
               std::string       host,
               int               time_out = 60);

    ~RTSPClient();

    bool add_stream(IAVStream * stream, uint16_t host_port, std::string ip = "", uint16_t port = 0);

    bool add_stream(IAVStream * stream, std::shared_ptr<IUTETransport> transport, uint8_t *channelid);

    void del_stream(IAVStream * stream);

    void active_stream(IAVStream * stream);

    const int stream_size() const { return m_streams.size(); }

    std::string session();

    const uint32_t id() const { return m_id; }

    std::string host() const { return m_host; }

private:

    /* AV stream callback */
    void on_packet(IAVStream * stream, IRtpPacket * packet);

    /* ute transport callbacks */
    void on_recv(std::shared_ptr<IUTETransport> transport, const char * data, int len) {/*XULOG_E("RTSPClient should not receve any data: %s", data);*/}

    void on_send(std::shared_ptr<IUTETransport> transport, int len) { /*XULOG_D("send %d bytes.", len);*/ }

    void on_message(std::shared_ptr<IUTETransport> transport, int code, const char * message, void * arg) {}

private:

    IRTSPClientSink           * m_sink;
    IUTE                      * m_ute;
    uint32_t                    m_id;
    std::vector<NetStream *>    m_streams;
    std::string                 m_session; ///< random number as the 'Session' field in response to RTSP 'Setup' method.
    std::string                 m_host;
    XUMutex                     m_lock;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___RTSPCLIENT_H___
