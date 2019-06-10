#include "XUTime.h"

#if defined(_MSC_VER)
#pragma warning(disable : 4786)
#endif

#if defined(_WIN32)
#include <windows.h>
#include <time.h>
#else /// _WIN32
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#endif
#include <string>

#if defined(_MSC_VER)
#if defined(_WIN32)
#pragma comment(lib, "winmm.lib")
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
////
struct JJTimeSpec
{
    time_t  tv_sec;     /** seconds **/
    long    tv_nsec;    /** and nanoseconds **/
};

static JJTimeSpec getPrivTime()
{
    /** return time since midnight (0 hour), January 1, 1970 **/
    struct JJTimeSpec time = {0, 0};

#if defined(_WIN32)
    uint64_t intervals;
    FILETIME ft;

    GetSystemTimeAsFileTime(&ft);

    /**
     * A file time is a 64-bit value that represents the number
     * of 100-nanosecond intervals that have elapsed since
     * January 1, 1601 12:00 A.M. UTC.
     *
     * Between January 1, 1970 (Epoch) and January 1, 1601 there were
     * 134744 days,
     * 11644473600 seconds or
     * 11644473600,000,000,0 100-nanosecond intervals.
     */
    intervals = ((uint64_t) ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    intervals -= 116444736000000000;

    time.tv_sec  = (long)(intervals / 10000000);
    time.tv_nsec = ((long)((intervals % 10000000) / 10)) * 1000;
#elif defined (CLOCK_REALTIME) && defined (PREFER_CLOCK_REALTIME)
    {
        struct timespec ts;
        clock_gettime (CLOCK_REALTIME, &ts);

        time.tv_sec  = ts.tv_sec;
        time.tv_nsec = ts.tv_nsec;
    }
#else
    {
        struct timeval tv;
        gettimeofday (&tv, NULL);

        time.tv_sec  = tv.tv_sec;
        time.tv_nsec = tv.tv_usec * 1000;
    }
#endif
    return time;
}

int64_t
XUGetTickCount64()
{
    /** return time in milliseconds **/
    JJTimeSpec time = getPrivTime();
    return time.tv_sec * 1000 + time.tv_nsec / 1000000;
}

void
XUSleep(unsigned long milliseconds)
{
    const int64_t te = XUGetTickCount64() + milliseconds;

    while(1)
    {
#if defined(_WIN32)
        ::Sleep(1);
#else
        usleep(500);
#endif

        if (XUGetTickCount64() >= te)
        {
            break;
        }
    }
}

void
XUGetTimeText(std::string &text)
{
    /** return the formatted time string **/
    JJTimeSpec time = getPrivTime();

    time_t curtime = time.tv_sec;
    struct tm *timeinfo = localtime(&curtime);

    char time_string[64] = {0};
    sprintf(time_string, "%.2d-%.2d %.2d:%.2d:%.2d:%03d",
            timeinfo->tm_mon + 1,
            timeinfo->tm_mday,
            timeinfo->tm_hour,
            timeinfo->tm_min,
            timeinfo->tm_sec,
            (int)(time.tv_nsec / 1000000));

    text = time_string;
}

uint64_t
XUNTPTime()
{
    JJTimeSpec time = getPrivTime();

    /** convert nanoseconds to 32-bits fraction (232 picosecond units) **/
    uint64_t t = (uint64_t)(time.tv_nsec) << 32;
    t /= 1000000000;

    /** There is 70 years (incl. 17 leap ones) offset to the Unix Epoch.
     *  No leap seconds during that period since they were not invented yet.
     */
    ///< assert (t < 0x100000000);
    t |= ((70LL * 365 + 17) * 24 * 60 * 60 + time.tv_sec) << 32;

    /** same as below:
     *  uint64_t msw = time.tv_sec + 0x83AA7E80; ///< 0x83AA7E80 is the number of seconds from 1900 to 1970
     *  uint64_t lsw = (uint32_t)((double)time.tv_nsec * (double)(((uint64_t)1) << 32) * 1.0e-9);
     *  t = msw << 32 | lsw;
     */
    return t;
}
/////////////////////////////////////////////////////////////////////////////
////
