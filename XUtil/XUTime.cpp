#include "XUTime.h"
#include "XUMutex.h"

#if defined(_MSC_VER)
#pragma warning(disable : 4786)
#endif

#if defined(_WIN32)
#include <windows.h>
#include <mmsystem.h>
#endif

#include <cassert>
#include <string>

#if defined(_MSC_VER)
#if defined(_WIN32)
#pragma comment(lib, "winmm.lib")
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
////

#if defined(_WIN32)
static unsigned long    g_s_tlsKey0 = (unsigned long)-1;
static unsigned long    g_s_tlsKey1 = (unsigned long)-1;
#endif
static XUMutex          g_s_lock;

/////////////////////////////////////////////////////////////////////////////
////

int64_t
XUGetTickCount64()
{
#if defined(_WIN32) 
    
    const uint32_t tick = ::timeGetTime();
    
    if (g_s_tlsKey0 == (unsigned long)-1)
    {
        g_s_lock.lock();
        
        if (g_s_tlsKey0 == (unsigned long)-1) //// double check
        {
            g_s_tlsKey0 = ::TlsAlloc(); //// dynamic TLS!!!
        }
        
        g_s_lock.unlock();
    }
    
    if (g_s_tlsKey1 == (unsigned long)-1)
    {
        g_s_lock.lock();
        
        if (g_s_tlsKey1 == (unsigned long)-1) //// double check
        {
            g_s_tlsKey1 = ::TlsAlloc(); //// dynamic TLS!!!
        }
        
        g_s_lock.unlock();
    }
    
    uint32_t oldTick0 = (uint32_t)::TlsGetValue(g_s_tlsKey0);
    uint32_t oldTick1 = (uint32_t)::TlsGetValue(g_s_tlsKey1);
    
    if (tick > oldTick0)
    {
        oldTick0 = tick;
        ::TlsSetValue(g_s_tlsKey0, (void*)oldTick0);
    }
    else if (tick < oldTick0)
    {
        oldTick0 = tick;
        ++oldTick1;
        ::TlsSetValue(g_s_tlsKey0, (void*)oldTick0);
        ::TlsSetValue(g_s_tlsKey1, (void*)oldTick1);
    }
    else
    {
    }
    
    int64_t ret = oldTick1;
    ret <<= 32;
    ret |= oldTick0;
    
    return ret;
    
#elif defined(XU_LACKS_CLOCK_GETTIME) //// for MacOS!!!
    
    struct timespec tick;
    clock_gettime(CLOCK_MONOTONIC, &tick);
    
    int64_t ret = tick.tv_sec;
    ret *= 1000;
    ret += tick.tv_nsec / 1000000;
    
    return ret;
    
#else
    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    int64_t ret = tv.tv_sec;
    ret *= 1000;
    ret += tv.tv_usec / 1000;
    
    return ret;
    
#endif
}

void
XUSleep(unsigned long milliseconds)
{
#if defined(_WIN32) 
    ::Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

void
XUGetTimeText(std::string &text)
{
#if defined(_WIN32)
    SYSTEMTIME st;
    ::GetLocalTime(&st);

    char timeText[256] = "";
    sprintf_s(timeText,
              "%04d.%02d.%02d --- %02d:%02d:%02d",
              (int)st.wYear,
              (int)st.wMonth,
              (int)st.wDay,
              (int)st.wHour,
              (int)st.wMinute,
              (int)st.wSecond);

#else //// WIN32, _WIN32_WCE

    struct tm theTm;
    memset(&theTm, 0, sizeof(struct tm));

    const time_t     theTime = time(NULL);
    struct tm* const theTm2 = localtime(&theTime);
    if (theTm2 != NULL)
    {
        theTm = *theTm2;
        theTm.tm_year += 1900;
        theTm.tm_mon += 1;
    }

    char timeText[256] = "";
    sprintf(timeText,
            "%04d.%02d.%02d --- %02d:%02d:%02d",
            (int)theTm.tm_year,
            (int)theTm.tm_mon,
            (int)theTm.tm_mday,
            (int)theTm.tm_hour,
            (int)theTm.tm_min,
            (int)theTm.tm_sec);
#endif //// WIN32, _WIN32_WCE

    text = timeText;
}

uint64_t
XUNTPTime()
{
    /* system time as 100ns since Jan 1, 1601 */
    FILETIME ft;
    ULARGE_INTEGER li;

    GetSystemTimeAsFileTime(&ft);
    
    li.HighPart = ft.dwHighDateTime;
    li.LowPart  = ft.dwLowDateTime;

    double dNTP = double(li.QuadPart) / (1000 * 10000); ///< convert to seconds as double

    /* 1601 to 1900 is 299*365 days, plus 72 leap years (299/4 minus 1700, 1800) */
    const int64_t DAYS_FROM_FILETIME_TO_NTP = 109207;
    int64_t offsetSeconds = DAYS_FROM_FILETIME_TO_NTP * 24 * 60 * 60;
    dNTP -= offsetSeconds;

    double fp = (dNTP * (1LL << 32)); ///< return as 64-bit 32.32 fixed point

    /* simple cast of ULONGLONG ntp = static_cast<ULONGLONG>(fp) fails on Visual Studio if
       the value is greater than INT64_MAX. So half the value before casting.
     */
    uint64_t ntp = (uint64_t)(fp / 2);
    ntp += ntp;

    return ntp;
}

/////////////////////////////////////////////////////////////////////////////
////
