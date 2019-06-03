#include "RtpPacket.h"

#include <windows.h>
#include <WinSock2.h>
#include <new>

#define MAX_PAYLOAD_BYTES (1024 * 63)

/////////////////////////////////////////////////////////////////////////////
////

RtpPacket *
RtpPacket::create_instance(const void * payload, uint16_t size)
{
    RtpPacket * packet = NULL;
    if (payload == NULL || size == 0 || size > MAX_PAYLOAD_BYTES)
    {
        return NULL;
    }

    packet = new(std::nothrow)RtpPacket();
    if (!packet || !packet->init(payload, size))
    {
        delete packet;
        packet = NULL;
    }

    return packet;
}

RtpPacket::RtpPacket() : m_packet(NULL)
{

}

RtpPacket::~RtpPacket()
{
    delete[] (char *)m_packet;
}

bool
RtpPacket::init(const void * payload, uint16_t size)
{
    m_packet = (RTP_PACKET *)new(std::nothrow) char[sizeof(RTP_PACKET) + size - 1];
    if (!m_packet)
    {
        return false;
    }

    memset(m_packet, 0, sizeof(RTP_PACKET));

    m_packet->hdr.v = 2;

    if (payload != NULL)
    {
        memcpy(m_packet->payload, payload, size);
        m_packet->payload_size = size;
    }

    return true;
}

void
RtpPacket::set_marker(bool m)
{
    m_packet->hdr.m = m ? 1 : 0;
}

bool
RtpPacket::marker() const
{
    return m_packet->hdr.m != 0;
}

void
RtpPacket::set_payload_type(char pt)
{
    m_packet->hdr.pt = pt;
}

char
RtpPacket::payload_type() const
{
    return m_packet->hdr.pt;
}

void
RtpPacket::set_sequence(uint16_t seq)
{
    m_packet->hdr.seq = htons(seq);
}

uint16_t
RtpPacket::sequence() const
{
    return ntohs(m_packet->hdr.seq);
}

void
RtpPacket::set_timestamp(uint32_t ts)
{
    m_packet->hdr.ts = htonl(ts);
}

uint32_t
RtpPacket::timestamp() const
{
    return ntohl(m_packet->hdr.ts);
}

void
RtpPacket::set_ssrc(uint32_t ssrc)
{
    m_packet->hdr.ssrc = htonl(ssrc);
}

uint32_t
RtpPacket::ssrc() const
{
    return ntohl(m_packet->hdr.ssrc);
}

char *
RtpPacket::payload()
{
    return m_packet->payload;
}

uint16_t
RtpPacket::payload_size() const
{
    return m_packet->payload_size;
}

const char *
RtpPacket::rtp_packet()
{
    return (char *)(&m_packet->hdr);
}

uint16_t 
RtpPacket::rtp_packet_size()
{
    return sizeof(m_packet->hdr) + m_packet->payload_size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////
