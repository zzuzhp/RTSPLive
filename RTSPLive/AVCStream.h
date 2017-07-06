#ifndef ___AVCSTREAM_H___
#define ___AVCSTREAM_H___

#include "AVStream.h"
#include "XUtil/XURefCount.h"
#include "XUtil/XUStat.h"

#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
////

class AVCStream : public VideoStream,
                  public XURefCount
{
public:

    static AVCStream * create_instance(uint32_t id, RTSP_avc_init_param *init_param);

    unsigned long addRef()
    {
        unsigned long ref = XURefCount::addRef();
        return ref;
    }

    unsigned long release()
    {
        unsigned long ref = XURefCount::release();
        return ref;
    }

    void push_frame(char * data, int len, uint32_t ts);

    std::vector<std::string> sps_pps();

    int profile_level_id();

    bool packed() const { return true; /* we force RFC packets */ }

private:

    AVCStream(uint32_t id, RTSP_avc_init_param *init_param);

    ~AVCStream();

    void send_nalu(const char * nalu, int len, uint32_t ts, bool marker);

    void send_packet(const char * data, int len, uint32_t ts, bool marker);

private:

    std::string     m_sps;
    std::string     m_pps;
    RTSP_AVC_PACK   m_pack;
    char          * m_fua_packet;
    bool            m_first;
    const int       m_max_packet_size;
    XUStatBitRate   m_framerate;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___AVCSTREAM_H___
