#include "AACStream.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////

AACStream *
AACStream::create_instance(uint32_t id)
{
    AACStream * stream = new AACStream(id);
    return stream;
}

AACStream::AACStream(uint32_t id) : AudioStream(id)
{
    m_type = RTSP_MEDIA_AAC;
    m_name = "mpeg4-generic";   ///< rfc6416 ///< MP4A-LATM

    m_frequency = 44100;
    m_channels  = 2;
    m_precision = 16;
}

AACStream::~AACStream()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
////
