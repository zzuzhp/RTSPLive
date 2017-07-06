#include "XUStat.h"
#include "XUTime.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////

XUStatBitRate::XUStatBitRate()
{
    m_timeSpan  = 5;
    m_startTick = 0;
    m_bits      = 0;
    m_bitRate   = 0;
}

XUStatBitRate::~XUStatBitRate()
{
}

void
XUStatBitRate::set_timeSpan(unsigned long timeSpanInSeconds) /* = 5 */
{
    if (timeSpanInSeconds == 0)
    {
        return;
    }

    m_timeSpan = timeSpanInSeconds;
}

void
XUStatBitRate::push_data(unsigned long dataBits)
{
    if (m_startTick == 0)
    {
        m_startTick = XUGetTickCount64() - m_timeSpan * 1000 / 2; //// 50%!!!
    }

    m_bits += dataBits;

    update();
}

float
XUStatBitRate::bitrate() const
{
    return m_bitRate;
}

void
XUStatBitRate::update()
{
    if (m_startTick == 0)
    {
        return;
    }

    const int64_t tick = XUGetTickCount64();

    if (tick - m_startTick >= 1000)
    {
        m_bitRate = m_bits * 1000 / (uint32_t)(tick - m_startTick);
    }

    if (tick - m_startTick >= m_timeSpan * 1000)
    {
        m_startTick = tick - m_timeSpan * 1000 / 2; //// 50%!!!
        m_bits = m_bitRate * (uint32_t)(tick - m_startTick) / 1000;
    }
}

void
XUStatBitRate::reset()
{
    m_startTick = 0;
    m_bits      = 0;
    m_bitRate   = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////
