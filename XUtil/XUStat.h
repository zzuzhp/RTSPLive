#ifndef ___XUSTAT_H___
#define ___XUSTAT_H___

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
////

class XUStatBitRate
{
public:

    XUStatBitRate();

    virtual ~XUStatBitRate();

    void set_timeSpan(unsigned long timeSpanInSeconds = 5);

    void push_data(unsigned long dataBits);

    float bitrate() const;

    void update();

    void reset();

private:

    int64_t     m_timeSpan;
    int64_t     m_startTick;
    float       m_bits;
    float       m_bitRate;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___XUSTAT_H___
