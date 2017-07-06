#ifndef ___RTSPRENDERER_H___
#define ___RTSPRENDERER_H___

#include "RTSPLive/RTSPLive.h"

#include <streams.h>
#include <string>

////////////////////////////////////////////////////////////////////////////////////////////////////
////

class RTSPRenderer : public CBaseRenderer
{
public:

    static CUnknown * WINAPI create_instance(LPUNKNOWN punk, HRESULT *phr);

    HRESULT OnStartStreaming();
    HRESULT OnStopStreaming();

private:

    RTSPRenderer(TCHAR *name, LPUNKNOWN punk, HRESULT *phr);

    ~RTSPRenderer();

protected:

    HRESULT CheckMediaType(const CMediaType *pmt);

    HRESULT DoRenderSample(IMediaSample * pMediaSample);

    HRESULT SetMediaType(const CMediaType * pmt);
#if 0
    HRESULT ShouldDrawSampleNow(IMediaSample * pMediaSample, REFERENCE_TIME * pStartTime, REFERENCE_TIME * pEndTime);
#endif
private:

    CMediaType              m_mt;
    RTSP_avc_init_param   * m_avc_param;
    uint32_t                m_stream;
    IRTSPLive             * m_rtsp;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< 
