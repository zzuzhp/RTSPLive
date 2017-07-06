#ifndef ___XUTHREAD_H___
#define ___XUTHREAD_H___

#include "XUMutex.h"

#if defined(_MSC_VER)
#pragma warning(disable : 4786)
#endif

#include <vector>

/////////////////////////////////////////////////////////////////////////////
////

class XUThreadBase
{
protected:
    
    XUThreadBase();
    
    virtual ~XUThreadBase();
    
    bool spawn(bool realtime = false);
    
    void wait();
    
    virtual void svc() = 0;
    
private:
    
#if defined(_WIN32_WCE)
    static unsigned long __stdcall svcRun(void * arg);
#elif defined(WIN32)
    static unsigned int __stdcall svcRun(void * arg);
#else
    static void * svcRun(void * arg);
#endif
    
private:
    
    unsigned long    m_threadCount;
    bool             m_realtime;
    XUMutexCondition m_cond;
    XUMutex          m_lock;
};

/////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___XUTHREAD_H___
