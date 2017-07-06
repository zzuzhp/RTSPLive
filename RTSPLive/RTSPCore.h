#ifndef ___RTSPCORE_H___
#define ___RTSPCORE_H___

#include "RTSPLive.h"
#include "UTE/UTE.h"
#include "SDPEncoder.h"
#include "RTSPMessager.h"
#include "RTSPClient.h"
#include "XUtil/XUMutex.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////

class RTSPCore : public IRTSPLive,
                 public IRTSPMessagerSink,
                 public IRTSPClientSink
{
public:

    RTSPCore();

    ~RTSPCore();

    bool build(uint16_t listen_port);

    void tear();

    bool start();

    void stop();

    uint32_t add_stream(RTSP_MEDIA type, void * init_param);

    void push_stream(uint32_t stream, RTSP_media_frame * frame);

private:

    RTSPClient * get_client(uint32_t id);

    void remove_client(uint32_t id);

    uint16_t get_rtp_port() { return m_portbase += 2, m_portbase - 2; }

    /* Messager callbacks */
    uint32_t on_new_client(std::string ip, uint16_t port);

    void on_rtsp_options(uint32_t clientId, RTSPRequest * request);

    void on_rtsp_describe(uint32_t clientId, RTSPRequest * request);

    void on_rtsp_setup(uint32_t clientId, RTSPRequest * request, RTSP_Setup * param);

    void on_rtsp_play(uint32_t clientId, RTSPRequest * request, RTSP_Play * param);

    void on_rtsp_teardown(uint32_t clientId, RTSPRequest * request, RTSP_Teardown * param);

    /* client callbacks */
    void on_time_out(RTSPClient * client);

private:

    IUTE                        * m_ute;        ///< send/receive media streams
    RTSPMessager                * m_messager;   ///< send/receive/parse rtsp messages
    SDPEncoder                  * m_sdpenc;
    std::vector<IAVStream *>      m_streams;
    std::vector<RTSPClient *>     m_clients;
    uint8_t                       m_pt;
    uint32_t                      m_streamId;
    uint32_t                      m_clientId;
    uint16_t                      m_portbase;
    XUMutex                       m_lock;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___RTSPCORE_H___

