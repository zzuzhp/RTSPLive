#ifndef ___XUTIME_H___
#define ___XUTIME_H___

#if defined(_MSC_VER)
#pragma warning(disable : 4786)
#endif

#include <string>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////
////

int64_t
XUGetTickCount64();

void
XUSleep(unsigned long milliseconds);

void
XUGetTimeText(std::string &text);

uint64_t
XUNTPTime();

/////////////////////////////////////////////////////////////////////////////
////

#ifdef __cplusplus
}
#endif

#endif ///< ___XUTIME_H___
