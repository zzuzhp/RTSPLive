#include "AVCStream.h"
#include "RtpPacket.h"
#include "XUtil/XULog.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////

AVCStream * 
AVCStream::create_instance(uint32_t id, RTSP_avc_init_param *init_param)
{
    AVCStream * stream = new AVCStream(id, init_param);
    return stream;
}

AVCStream::AVCStream(uint32_t id, RTSP_avc_init_param *init_param) : VideoStream(id),
                                                                     m_first(true),
                                                                     m_pack(RTSP_AVC_RFC),
                                                                     m_fua_packet(nullptr),
                                                                     m_max_packet_size(1200)
{
    m_type = RTSP_MEDIA_AVC; 
    m_name = "H264"; ///< rfc6184
 
    if (init_param)
    {
        if (init_param->sps_len > 0)
        {
            m_sps.append(init_param->sps, init_param->sps + init_param->sps_len);
        }

        if (init_param->pps_len > 0)
        {
            m_pps.append(init_param->pps, init_param->pps + init_param->pps_len);
        }

        m_pack = init_param->pack;
    }

    if (m_pack != RTSP_AVC_RFC)
    {
        m_fua_packet = new char[m_max_packet_size];
    }
}

AVCStream::~AVCStream()
{
    delete[] m_fua_packet;
}

void 
AVCStream::push_frame(char * data, int len, uint32_t ts)
{
    m_framerate.push_data(1);
    
    if (++m_frames % 300 == 0)
    {
        XULOG_D("AVCStream input framerate: %f.", m_framerate.bitrate());
    }

    if (m_first)
    {
        send_nalu(m_sps.c_str(), m_sps.length(), ts, false);
        send_nalu(m_pps.c_str(), m_pps.length(), ts, false);

        m_first = false;
    }

    if (m_pack == RTSP_AVC_RFC)
    {
        //send_packet(data, len, ts, );
    }
    else if (m_pack == RTSP_AVC_ANNEXB)
    {
        int start = -1;
        for (int i = 0; i < len - 3;)
        {
            if (data[i] == 0 && data[i + 1] == 0 && data[i + 2] == 1)
            {
                int cur = i + 3;
                if (start == -1)
                {
                    start = cur;
                    i += 3;

                    continue;
                }
                else
                {
                    send_nalu(&data[start], i - start - (data[i - 1] == 0 ? 1 : 0), ts, false);
                    start = cur;
                }
            }

            ++i;
        }

        if (start != -1)
        {
            send_nalu(&data[start], len - start, ts, true);
        }
    }
    else
    {
        int len_bytes = (m_pack == RTSP_AVC_AVCC_1 ? 1 : (m_pack == RTSP_AVC_AVCC_2 ? 2 : 4));
        for (int i = 0; i < len;)
        {
            unsigned char * unit = (unsigned char *)data + i;
            int nalu_size = 0;
            for (int j = 0; j < len_bytes; ++j)
            {
                nalu_size |= (unit[j] << ((len_bytes - j - 1) << 3));
            }

            i += len_bytes;
            i += nalu_size;

            send_nalu((char *)unit + len_bytes, nalu_size, ts, i >= len);
        }
    }
}

void 
AVCStream::send_nalu(const char * nalu, int len, uint32_t ts, bool marker)
{
#if 0
    static FILE * file = fopen("stream.264", "wb");
    fwrite("\x00\x00\x00\x01", 1, 4, file);
    fwrite(nalu, 1, len, file);
    fflush(file);
#endif

    /* for simplicity: we use only Single NALU Packet or FUA */
    if (len < m_max_packet_size)
    {
        /* single nalu packet */
        send_packet(nalu, len, ts, marker);
    }
    else
    {
        /* FUA */
        const char    * data     = nalu;
        int             data_len = len;
        unsigned char   header   = data[0];

        ++data;
        --data_len;

        do
        {
            int pkt_len = (m_max_packet_size - 2) > data_len ? data_len : (m_max_packet_size - 2);
            
            m_fua_packet[0] = (header & 0xe0) + 28; ///< 28: FUA

            BYTE naltype = header & 0x1f;
            if (data_len == len - 1)
            {
                naltype |= 0x80;    ///< first fragment of this NALU
            }
            else if (data_len == pkt_len)
            {
                naltype |= 0x40;    ///< last fragment of this NALU
            }

            m_fua_packet[1] = naltype;

            memcpy(m_fua_packet + 2, data, pkt_len);

            data += pkt_len;
            data_len -= pkt_len;

            send_packet(m_fua_packet, pkt_len + 2, ts, (data_len == 0) && marker);
        } while (data_len > 0);
    }
}

void 
AVCStream::send_packet(const char * data, int len, uint32_t ts, bool marker)
{
#if 0
    static FILE * file = fopen("stream.pkt", "wb");
    fwrite(&marker, 1, 1, file);
    fwrite(&len, 1, 4, file);
    fwrite(data, 1, len, file);
    fflush(file);
#endif

    IRtpPacket * packet = RtpPacket::create_instance(data, len);

    if (packet)
    {
        packet->set_payload_type(m_pt);
        packet->set_sequence(m_seqno++);
        packet->set_timestamp(ts * 90);
        packet->set_marker(marker);

        on_rtp_packet(packet);

        delete (RtpPacket *)packet;
    }
}

std::vector<std::string> 
AVCStream::sps_pps()
{
    std::vector<std::string> param_sets;
    if (!m_sps.empty() && !m_pps.empty())
    {
        param_sets.push_back(m_sps);
        param_sets.push_back(m_pps);
    }

    return param_sets;
}

int
AVCStream::profile_level_id()
{
   /*  RFC6184 -- RTP Payload Format for H.264 Video
    8.1.  Media Type Registration
    A base16 [7] (hexadecimal) representation of the following
    three bytes in the sequence parameter set NAL unit is specified
    in [1]:
    1) profile_idc,
    2) a byte herein referred to as
    profile-iop, composed of the values of constraint_set0_flag,
    constraint_set1_flag, constraint_set2_flag,
    constraint_set3_flag, constraint_set4_flag,
    constraint_set5_flag, and reserved_zero_2bits in bit-
    significance order, starting from the most-significant bit, and
    3) level_idc.
    */

    return ((unsigned char)m_sps[1] << 16) | ((unsigned char)m_sps[2] << 8) | (unsigned char)m_sps[3];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////
