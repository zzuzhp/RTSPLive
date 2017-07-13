#ifndef ___SDPENCODER_H___
#define ___SDPENCODER_H___

#include "AVStream.h"
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
//// SDP(RFC4566)

/* £¼type£¾=<value>[CRLF]
    type with '*' is optioanl, only v/o/s/t/m are mandatory

Sessiondescription
    v=  (protocol version)
    o=  (owner/creator and session identifier).
    s=  (session name)
    i=* (session information)
    u=* (URI of description)
    e=* (email address)
    p=* (phone number)
    c=* (connection information - notrequired if included in all media)
    b=* (bandwidth information)
    One or more time descriptions (seebelow)
    z=* (time zone adjustments)
    k=* (encryption key)
    a=* (zero or more session attributelines)
    Zero or more media descriptions (seebelow)
 
Time description
    t=  (time the session is active)
    r=* (zero or more repeat times)
 
Media description
    m=  (media name and transport address)
    i=* (media title)
    c=* (connection information - optionalif included at session-level)
    b=* (bandwidth information)
    k=* (encryption key)
    a=* (zero or more media attributelines)
 */

class SDPEncoder
{
public:

    SDPEncoder();

    bool add_media(IAVStream * stream, uint8_t pt);

    std::string sdp() const { return m_sdp; }

private:

    void add_line(std::vector<std::string> attrs);

    int base64_decode(char * bufplain, const char * bufcoded);

    std::string base64_encode(const char * string, int len);

private:

    std::string m_sdp;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___SDPENCODER_H___
