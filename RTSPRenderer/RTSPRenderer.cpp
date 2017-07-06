#include "RTSPRenderer.h"

#include <fourcc.h>
#include <Dvdmedia.h>
#include <cassert>
#include <initguid.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
////

// {2AA165F3-77AF-4674-8E28-1E6133305CCF}
DEFINE_GUID(CLSID_RTSPRenderer,
    0x2aa165f3, 0x77af, 0x4674, 0x8e, 0x28, 0x1e, 0x61, 0x33, 0x30, 0x5c, 0xcf);

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_Video, 
    &MEDIASUBTYPE_NULL,
};

const AMOVIESETUP_PIN sudPins =
{
    L"video",           ///< pin string name
    FALSE,              ///< is it rendered
    FALSE,              ///< is it an output
    FALSE,              ///< Allowed none
    FALSE,              ///< likewise many
    &CLSID_NULL,        ///< connects to filter
    L"Output",          ///< connects to pin
    1,                  ///< Number of types
    &sudPinTypes        ///< Pin info
};

const AMOVIESETUP_FILTER sudFilter =
{
    &CLSID_RTSPRenderer,///< filter clsid
    L"RTSP Renderer",   ///< filter name
    MERIT_NORMAL,       ///< filter merit
    1,                  ///< number pins
    &sudPins            ///< pin details
};

//
//  Object creation stuff
//
CFactoryTemplate g_Templates[] = 
{
    L"Dump", &CLSID_RTSPRenderer, RTSPRenderer::create_instance, NULL, &sudFilter
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

//
// CreateInstance
//
// Provide the way for COM to create the filter
//
CUnknown * 
WINAPI 
RTSPRenderer::create_instance(LPUNKNOWN punk, HRESULT *phr)
{
    RTSPRenderer * renderer = new RTSPRenderer(L"RTSP Renderer", punk, phr);
    return renderer;
}

RTSPRenderer::RTSPRenderer(TCHAR *name, LPUNKNOWN punk, HRESULT *phr) : CBaseRenderer(CLSID_RTSPRenderer, name, punk, phr)
{
    m_avc_param = new RTSP_avc_init_param;
    memset(m_avc_param, 0, sizeof(RTSP_avc_init_param));

    m_rtsp = RTSPLiveCreate(8554);
}

RTSPRenderer::~RTSPRenderer()
{
    RTSPLiveDestroy(m_rtsp);

    if (m_avc_param)
    {
        if (m_avc_param->sps)
        {
            delete[] m_avc_param->sps;
        }

        if (m_avc_param->pps)
        {
            delete[] m_avc_param->pps;
        }

        delete m_avc_param;
    }
}

HRESULT
RTSPRenderer::CheckMediaType(const CMediaType *pmt)
{
    /// Check the major type is MEDIATYPE_Video
    const GUID *pMajorType = pmt->Type();
    if (*pMajorType != MEDIATYPE_Video)
    {
        return E_INVALIDARG;
    }

    const GUID *pFormatType = pmt->FormatType();
    if (*pFormatType == FORMAT_MPEG2Video)
    {
        MPEG2VIDEOINFO * pvi = (MPEG2VIDEOINFO*)pmt->Format();
        if (pvi->dwFlags == 1 || pvi->dwFlags == 2 || pvi->dwFlags == 4)
        {
            /* sps_len(2 bytes) + sps + pps_len(2 bytes) + pps */ /*bigendian*/
            const char *unit, *sps, *pps;
            const int len_bytes = 2;

            if (m_avc_param->sps)
            {
                delete[] m_avc_param->sps;
                m_avc_param->sps = nullptr;
                m_avc_param->sps_len = 0;
            }

            if (m_avc_param->pps)
            {
                delete[] m_avc_param->pps;
                m_avc_param->pps = nullptr;
                m_avc_param->pps_len = 0;
            }

            unit = (const char *)(&pvi->dwSequenceHeader);
            sps  = unit + len_bytes;

            m_avc_param->sps_len = (unit[0] << 8) + unit[1];

            m_avc_param->sps = new char[m_avc_param->sps_len];
            memcpy(m_avc_param->sps, sps, m_avc_param->sps_len);

            unit = sps + m_avc_param->sps_len;
            pps = unit + len_bytes;

            m_avc_param->pps_len = (unit[0] << 8) + unit[1];

            m_avc_param->pps = new char[m_avc_param->pps_len];
            memcpy(m_avc_param->pps, pps, m_avc_param->pps_len);

            m_avc_param->pack = pvi->dwFlags == 1 ? RTSP_AVC_AVCC_1 : (pvi->dwFlags == 2 ? RTSP_AVC_AVCC_2 : RTSP_AVC_AVCC_4);
        }
    }

    /// we support h264
    if (FOURCCMap(&(GUID)*pmt->Subtype()).GetFOURCC() == MAKEFOURCC('H', '2', '6', '4') ||
        FOURCCMap(&(GUID)*pmt->Subtype()).GetFOURCC() == MAKEFOURCC('h', '2', '6', '4') ||
        FOURCCMap(&(GUID)*pmt->Subtype()).GetFOURCC() == MAKEFOURCC('A', 'V', 'C', '1') ||
        FOURCCMap(&(GUID)*pmt->Subtype()).GetFOURCC() == MAKEFOURCC('a', 'v', 'c', '1'))
    {
        return S_OK;
    }

    return S_FALSE;
}

HRESULT
RTSPRenderer::SetMediaType(const CMediaType * pmt)
{
    if (FAILED(CheckMediaType(pmt)))
    {
        return E_FAIL;
    }

    m_mt = *pmt;
    return CBaseRenderer::SetMediaType(pmt);
}

HRESULT 
RTSPRenderer::OnStartStreaming()
{
    if (!m_rtsp)
    {
        return E_FAIL;
    }

    m_stream = m_rtsp->add_stream(RTSP_MEDIA_AVC, m_avc_param);
    m_rtsp->start();

    return S_OK;
}

HRESULT 
RTSPRenderer::OnStopStreaming()
{
    if (!m_rtsp)
    {
        return E_FAIL;
    }

    m_rtsp->stop();

    return S_OK;
}

HRESULT
RTSPRenderer::DoRenderSample(IMediaSample *pSample)
{
    PBYTE data;
    REFERENCE_TIME tsTimeStart = 0, tsTimeEnd;
    RTSP_media_frame frame;

    DWORD len = pSample->GetActualDataLength();    
    pSample->GetPointer(&data);

    if (VFW_E_SAMPLE_TIME_NOT_SET == pSample->GetTime(&tsTimeStart, &tsTimeEnd))
    {
        tsTimeStart = 0;
    }
    
    frame.frame = (char *)data;
    frame.len   = len;
    frame.ts_ms = tsTimeStart / 10000;
    frame.type  = RTSP_FRAME_UNCERTAIN;

    m_rtsp->push_stream(m_stream, &frame);
    return S_OK;
}

#if 0
HRESULT
RTSPRenderer::ShouldDrawSampleNow(IMediaSample   * pMediaSample,
                                  REFERENCE_TIME * pStartTime,
                                  REFERENCE_TIME * pEndTime)
{
    /// Do not schedule, we want the most up-to-date frame.
    ///< !!! warning: this will let the demuxer push frame as fast as it can, ignoring the framerate
    return S_OK;
}
#endif

////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

//
// DllRegisterSever
//
// Handle the registration of this filter
//
STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);

} // DllRegisterServer

//
// DllUnregisterServer
//
STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);

} // DllUnregisterServer

//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
    return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////
