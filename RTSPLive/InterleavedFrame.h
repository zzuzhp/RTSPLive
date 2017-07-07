#ifndef ___INTERLEAVEDFRAME_H___
#define ___INTERLEAVEDFRAME_H___

#include "RtpPacket.h"
#include <string>

////////////////////////////////////////////////////////////////////////////////////////////////////
////

class InterleavedFrame
{
public:

    static int format(uint8_t ch, IRtpPacket *packet, char *frame)
    {
        /* add interleaved frame overhead */
        frame[0] = m_leadingbyte;
        frame[1] = ch;
        frame[2] = ((packet->rtp_packet_size() & 0xff00) >> 8);
        frame[3] = (packet->rtp_packet_size() & 0xff);

        memcpy(&frame[4], packet->rtp_packet(), packet->rtp_packet_size());

        return packet->rtp_packet_size() + 4;
    }

    static std::string parse(const char *data, int len)
    {
        if (len < m_overhead)
        {
            return "";
        }

        if (data[0] != m_leadingbyte)
        {
            return "";
        }

        return std::string(data + m_overhead, len - m_overhead);
    }

    static int overhead() { return m_overhead; }

private:
    /* 10.12 Stream data such as RTP packets is encapsulated by an ASCII dollar
       sign (24 hexadecimal), followed by a one-byte channel identifier,
       followed by the length of the encapsulated binary data as a binary,
       two-byte integer in network byte order.
     */
    static const int  m_overhead    = 4; ///< 1/*'$'*/ + 1/*ch id*/ + 2/*packet size*/
    static const char m_leadingbyte = '$';
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___INTERLEAVEDFRAME_H___
