#ifndef ___AVSTREAM_H___
#define ___AVSTREAM_H___

#include "RTSPLive.h"
#include "XUtil/XUMutex.h"

#include <string>
#include <list>

////////////////////////////////////////////////////////////////////////////////////////////////////
////

class IRtpPacket;
class IAVStream;

class IAVStreamObserver
{
public:
    virtual void on_packet(IAVStream * stream, IRtpPacket * packet) = 0;
};

class IAVStream
{
public:
    
    virtual ~IAVStream(){}

    virtual const RTSP_MEDIA media_type() const = 0;

    virtual const uint32_t stream_id() const = 0;

    virtual const std::string name() const = 0;

    virtual void set_avp_type(char pt) = 0;

    virtual const char avp_type() const = 0;

    virtual const int duration() const = 0;

    virtual const int bitrate() const = 0;

    virtual void push_frame(char * data, int len, uint32_t ts) = 0;

    virtual bool add_observer(IAVStreamObserver * observer) = 0;

    virtual void remove_observer(IAVStreamObserver * observer) = 0;
};

class AVStream : public IAVStream
{
protected:

    AVStream(uint32_t id) : m_id(id),
                            m_pt(-1),
                            m_type(RTSP_MEDIA_NULL),
                            m_duration(0),
                            m_bitrate(0),
                            m_frames(0)
    {
        m_seqno = rand();
    }

    ~AVStream()
    {

    }

    const RTSP_MEDIA media_type() const { return m_type; }

    const uint32_t stream_id() const { return m_id; }

    const std::string name() const { return m_name; }
    
    void set_avp_type(char pt) { m_pt = pt; }

    const char avp_type() const { return m_pt; }

    const int duration() const { return m_duration; }

    const int bitrate() const { return m_bitrate; }

    bool add_observer(IAVStreamObserver * observer);

    void remove_observer(IAVStreamObserver * observer);

protected:

    void on_rtp_packet(IRtpPacket * packet);

protected:

    uint32_t                        m_id;
    std::string                     m_name;
    RTSP_MEDIA                      m_type;
    uint16_t                        m_seqno;
    uint8_t                         m_pt;
    int                             m_duration;
    int                             m_bitrate;
    int                             m_frames;
    std::list<IAVStreamObserver *>  m_observers;
    XUMutex                         m_lock;
};

class VideoStream : public AVStream
{
public:

    const int width() const { return m_width; }

    const int height() const { return m_height; }
    
    const float framerate() const { return m_framerate; }

    virtual bool packed() const { return false; }

protected:

    VideoStream(uint32_t id) : AVStream(id),
                               m_width(0),
                               m_height(0),
                               m_framerate(0.)
    {

    }

protected:

    int         m_width;
    int         m_height;
    float       m_framerate;
};

class AudioStream : public AVStream
{
public:

    const int frequency() const { return m_frequency; }

    const int channels() const { return m_channels; }

    const int precision() const { return m_precision; }

protected:

    AudioStream(uint32_t id) : AVStream(id),
                               m_frequency(0),
                               m_channels(0),
                               m_precision(0)
    {

    }

protected:

    int         m_frequency;
    int         m_channels;
    int         m_precision;
};

IAVStream *
create_stream(uint32_t id, RTSP_MEDIA type, void *init);

void
destroy_stream(IAVStream * stream);

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___AVSTREAM_H___
