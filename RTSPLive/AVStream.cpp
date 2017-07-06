#include "AVStream.h"
#include "AACStream.h"
#include "AVCStream.h"

#include "XUtil/XUMutex.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////

bool 
AVStream::add_observer(IAVStreamObserver * observer)
{
    XUMutexGuard mon(m_lock);

    m_observers.push_back(observer);

    return true;
}

void 
AVStream::remove_observer(IAVStreamObserver * observer)
{
    XUMutexGuard mon(m_lock);

    m_observers.remove(observer);
}

void 
AVStream::on_rtp_packet(IRtpPacket * packet)
{
    XUMutexGuard mon(m_lock);

    //TODO: deliver out of the lock
    std::list<IAVStreamObserver *>::iterator itr = m_observers.begin();
    for (; itr != m_observers.end(); ++itr)
    {
        (*itr)->on_packet(this, packet);
    }
}

IAVStream *
create_stream(uint32_t id, RTSP_MEDIA type, void *init)
{
    if (type == RTSP_MEDIA_AVC)
    {
        return AVCStream::create_instance(id, (RTSP_avc_init_param *)init);
    }
    else if (type == RTSP_MEDIA_AAC)
    {
        return AACStream::create_instance(id);
    }

    return nullptr;
}

void
destroy_stream(IAVStream * stream)
{
    delete stream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////
