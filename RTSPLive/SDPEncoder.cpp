#include "SDPEncoder.h"
#include "AACStream.h"
#include "AVCStream.h"
#include "XUtil/XUTime.h"

#include <sstream>

////////////////////////////////////////////////////////////////////////////////////////////////////
////

SDPEncoder::SDPEncoder()
{
    /* session description */
    
    /* 5.1.  Protocol Version ("v=")
     */
    add_line(std::vector<std::string>{"v=0"});

    /* 5.2.  Origin ("o=")
       o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>
     */
    add_line(std::vector<std::string>{"o=-",
                                      std::to_string(XUNTPTime()),
                                      std::to_string(XUNTPTime()),
                                      "IN",
                                      "IP4",
                                      "0.0.0.0"});

    /* 5.3.  Session Name ("s=")
       s=<session name> 
     */
    add_line(std::vector<std::string>{"s=RTSPLive"});

    /* 5.7.  Connection Data ("c=")
       c=<nettype> <addrtype> <connection-address> 
     */
    add_line(std::vector<std::string>{"c=IN IP4 0.0.0.0"});

    /* 5.9.  Timing ("t=")
       t=<start-time> <stop-time> 
     */
    add_line(std::vector<std::string>{"t=0 0"});
}

bool 
SDPEncoder::add_media(IAVStream * stream, uint8_t pt)
{
    /* media description */
    if (!stream)
    {
        return false;
    }

    {
        /* 5.14.  Media Descriptions ("m=")
           m=<media> <port> <proto> <fmt> ...
         */
        std::vector<std::string> m;
        if (stream->media_type() > RTSP_MEDIA_VIDEO)
        {
            m.push_back("m=video"); ///< media
        }
        else if (stream->media_type() > RTSP_MEDIA_AUDIO)
        {
            m.push_back("m=audio"); ///< media
        }

        m.push_back("0");                   ///< port
        m.push_back("RTP/AVP");             ///< proto RTP A/V Profile
        m.push_back(std::to_string(pt));    ///< fmt

        add_line(m);
    }
    
    /* a=control
        C.1.1 Control URL 
        This attribute may contain either relative and absolute URLs,
        following the rules and conventions set out in RFC 1808 [25].
        Implementations should look for a base URL in the following order:
        1.     The RTSP Content-Base field
        2.     The RTSP Content-Location field
        3.     The RTSP request URL

        If this attribute contains only an asterisk (*), then the URL is
        treated as if it were an empty embedded URL, and thus inherits the
        entire base URL.
     */
    add_line(std::vector<std::string>{"a=control:streamid=" + std::to_string(stream->stream_id())});

    /* a=rtpmap: */
    std::vector<std::string> a;
    a.push_back("a=rtpmap:" + std::to_string(pt));
    
    if (stream->media_type() == RTSP_MEDIA_AVC)
    {
        AVCStream * avc = (AVCStream *)stream;

        a.push_back(stream->name() + "/90000");
        add_line(a);

        std::string params = "packetization-mode=" + std::to_string(avc->packed());

        std::vector<std::string> param_sets = avc->sps_pps();
        if (param_sets.size() == 2)
        {
            std::string sps = param_sets[0];
            std::string pps = param_sets[1];

            char * sps_enc = new char[sps.length() * 2];
            int sps_enc_len = base64_encode(sps_enc, sps.c_str(), sps.length());

            char * pps_enc = new char[pps.length() * 2];
            int pps_enc_len = base64_encode(pps_enc, pps.c_str(), pps.length());

            std::ostringstream profile;
            profile << std::hex << std::uppercase << avc->profile_level_id();

            params += ";profile-level-id=" + profile.str() + ";" +
                      "sprop-parameter-sets=" + std::string(sps_enc) + "," + std::string(pps_enc);

            delete[] sps_enc;
            delete[] pps_enc;
        }

        /* a=fmtp: */
        add_line(std::vector<std::string>{"a=fmtp:" + std::to_string(pt), params});

        return true;
    }
    else if (stream->media_type() == RTSP_MEDIA_AAC)
    {
        AACStream * aac = (AACStream *)stream;

        a.push_back(stream->name() + "/" + std::to_string(aac->frequency()) + "/" + std::to_string(aac->channels()));
        add_line(a);

        //TODO: add 'fmtp'

        return true;
    }

    return false;
}

void 
SDPEncoder::add_line(std::vector<std::string> attrs)
{
#define SDP_SEP     " "
#define SDP_ENDLINE "\r\n"

    for (std::vector<std::string>::size_type i = 0; i < attrs.size(); ++i)
    {
        m_sdp += attrs[i];
        if (i < attrs.size() - 1)
        {
            m_sdp += SDP_SEP;
        }
    }

    m_sdp += SDP_ENDLINE;
}

int 
SDPEncoder::base64_decode(char * bufplain, const char * bufcoded)
{
    int nprbytes, nbytesdecoded;
    const unsigned char * bufin;
    unsigned char * bufout;

    /* aaaack but it's fast and const should make it shared text page. */
    static const unsigned char pr2six[256] =
    {
        /* ASCII table */
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
        64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
        64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
    };

    bufin = (const unsigned char *)bufcoded;
    while (pr2six[*(bufin++)] <= 63);
    nprbytes = (bufin - (const unsigned char *)bufcoded) - 1;
    nbytesdecoded = ((nprbytes + 3) / 4) * 3;

    bufout = (unsigned char *)bufplain;
    bufin = (const unsigned char *)bufcoded;

    while (nprbytes > 4)
    {
        *(bufout++) = (unsigned char)(pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
        *(bufout++) = (unsigned char)(pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
        *(bufout++) = (unsigned char)(pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);

        bufin += 4;
        nprbytes -= 4;
    }

    /* Note: (nprbytes == 1) would be an error, so just ingore that case */
    if (nprbytes > 1)
    {
        *(bufout++) = (unsigned char)(pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
    }

    if (nprbytes > 2)
    {
        *(bufout++) = (unsigned char)(pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
    }

    if (nprbytes > 3) 
    {
        *(bufout++) = (unsigned char)(pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
    }

    *(bufout++) = '\0';
    nbytesdecoded -= (4 - nprbytes) & 3;

    return nbytesdecoded;
}

int 
SDPEncoder::base64_encode(char * encoded, const char * string, int len)
{
    int i;
    char * p;
    static const char basis_64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    p = encoded;
    for (i = 0; i < len - 2; i += 3)
    {
        *p++ = basis_64[(string[i] >> 2) & 0x3F];
        *p++ = basis_64[((string[i] & 0x3) << 4) | ((int)(string[i + 1] & 0xF0) >> 4)];
        *p++ = basis_64[((string[i + 1] & 0xF) << 2) | ((int)(string[i + 2] & 0xC0) >> 6)];
        *p++ = basis_64[string[i + 2] & 0x3F];
    }

    if (i < len)
    {
        *p++ = basis_64[(string[i] >> 2) & 0x3F];
        if (i == (len - 1)) 
        {
            *p++ = basis_64[((string[i] & 0x3) << 4)];
            *p++ = '=';
        }
        else 
        {
            *p++ = basis_64[((string[i] & 0x3) << 4) | ((int)(string[i + 1] & 0xF0) >> 4)];
            *p++ = basis_64[((string[i + 1] & 0xF) << 2)];
        }

        *p++ = '=';
    }

    *p++ = '\0';

    return p - encoded;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////
