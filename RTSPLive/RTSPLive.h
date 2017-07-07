#ifndef ___RTSPLIVE_H___
#define ___RTSPLIVE_H___

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#ifndef RTSPLIVE_EXTERN_C
#ifdef __cplusplus
#define RTSPLIVE_EXTERN_C extern "C"
#else
#define RTSPLIVE_EXTERN_C
#endif
#endif

#if (defined WIN32 || defined _WIN32) && defined RTSPLIVE_EXPORTS
#define RTSPLIVE_DLLEXPORTS __declspec(dllexport)
#else
#define RTSPLIVE_DLLEXPORTS __declspec(dllimport)
#endif

#if defined WIN32 || defined _WIN32
#define RTSPLIVE_CDECL   __cdecl
#else
#define RTSPLIVE_CDECL
#endif

#ifndef RTSPLIVEAPI
#define RTSPLIVEAPI(rettype) RTSPLIVE_EXTERN_C RTSPLIVE_DLLEXPORTS rettype RTSPLIVE_CDECL
#endif

enum RTSP_MEDIA
{
    RTSP_MEDIA_NULL  = 0,

    RTSP_MEDIA_AUDIO = 1,
    RTSP_MEDIA_AAC   = 2,

    RTSP_MEDIA_VIDEO = 6,
    RTSP_MEDIA_AVC   = 7
};

enum RTSP_FRAME_TYPE
{
    RTSP_FRAME_UNCERTAIN = 0,
    RTSP_FRAME_INTRA     = 1, ///< aka. key frame
    RTSP_FRAME_INTER     = 2
};

enum RTSP_AVC_PACK
{
    RTSP_AVC_RFC    = 1, ///< not supported now
    RTSP_AVC_ANNEXB = 2,
    RTSP_AVC_AVCC_1 = 3,
    RTSP_AVC_AVCC_2 = 4,
    RTSP_AVC_AVCC_4 = 5
};

struct RTSP_avc_init_param
{
    /* info used in SDP */
    char * sps;
    int    sps_len;
    char * pps;
    int    pps_len;

    RTSP_AVC_PACK pack;
};

struct RTSP_media_frame
{
    char * frame;
    int    len;

    RTSP_FRAME_TYPE type;
    uint32_t ts_ms;         ///< pts in ms
};

class IRTSPLive
{
public:

    virtual uint32_t add_stream(RTSP_MEDIA type, void * init_param) = 0;

    virtual void push_stream(uint32_t stream, RTSP_media_frame * frame) = 0;

    virtual bool start() = 0;

    virtual void stop() = 0;
};

RTSPLIVEAPI(IRTSPLive *)
RTSPLiveCreate(uint16_t listening_port);

RTSPLIVEAPI(void)
RTSPLiveDestroy(IRTSPLive * live);

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___RTSPLIVE_H___
