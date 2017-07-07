#ifndef ___RTPPACKET_H___
#define ___RTPPACKET_H___

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#pragma pack(push, 1)
#pragma pack(pop)

struct RTP_HEADER
{
#if defined(WORDS_BIGENDIAN)
    unsigned char v  : 2;
    unsigned char p  : 1;
    unsigned char x  : 1;
    unsigned char cc : 4;

    unsigned char m  : 1;
    unsigned char pt : 7;
#else
    unsigned char cc : 4;   ///< the number of CSRC identifiers that follow the fixed header
    unsigned char x  : 1;   ///< whether fixed header is followed by the header extension(exactly one)
    unsigned char p  : 1;   ///< padding indicator
    unsigned char v  : 2;   ///< version of RTP

    unsigned char pt : 7;
    unsigned char m  : 1;
#endif

    uint16_t    seq;
    uint32_t    ts;
    uint32_t    ssrc;
};

#pragma pack(push, 1)
#pragma pack(pop)

struct RTP_PACKET
{
    uint16_t   payload_size;
    RTP_HEADER hdr;
    char       payload[1];
};

/////////////////////////////////////////////////////////////////////////////
////

class IRtpPacket
{
public:

    virtual void set_marker(bool m) = 0;

    virtual bool marker() const = 0;

    virtual void set_payload_type(char pt) = 0;

    virtual char payload_type() const = 0;

    virtual void set_sequence(uint16_t seq) = 0;

    virtual uint16_t sequence() const = 0;

    virtual void set_timestamp(uint32_t ts) = 0;

    virtual uint32_t timestamp() const = 0;

    virtual void set_ssrc(uint32_t ssrc) = 0;

    virtual uint32_t ssrc() const = 0;

    virtual char * payload() = 0;

    virtual uint16_t payload_size() const = 0;

    virtual const char * rtp_packet() = 0;

    virtual uint16_t rtp_packet_size() = 0;
};

class RtpPacket : public IRtpPacket
{
public:

    static RtpPacket * create_instance(const void * payload, uint16_t size);

    static const int max_payload_size() { return 1200; }

    static const int max_packet_size() { return sizeof(RTP_HEADER) + max_payload_size(); }

    ~RtpPacket();

    void set_marker(bool m);

    bool marker() const;

    void set_payload_type(char pt);

    char payload_type() const;

    void set_sequence(uint16_t seq);

    uint16_t sequence() const;

    void set_timestamp(uint32_t ts);

    uint32_t timestamp() const;

    void set_ssrc(uint32_t ssrc);

    uint32_t ssrc() const;

    char * payload();

    uint16_t payload_size() const;

    const char * rtp_packet();

    uint16_t rtp_packet_size();

private:

    RtpPacket();

    bool init(const void * payload, uint16_t size);

private:

    RTP_PACKET * m_packet;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___RTPPACKET_H___
