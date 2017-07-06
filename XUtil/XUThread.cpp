#include "XUThread.h"
#include "XUMutex.h"
#include "XUTime.h"

#if defined(_MSC_VER)
#pragma warning(disable : 4786)
#endif

#if defined(_WIN32_WCE)
#include <windows.h>
#elif defined(WIN32)
#include <windows.h>
#include <process.h>
#include <time.h>
#else
#include <pthread.h>
#endif

#include <vector>

/////////////////////////////////////////////////////////////////////////////
////

#if defined(WIN32) || defined(_WIN32_WCE)

#if !defined(STACK_SIZE_PARAM_IS_A_RESERVATION)
#define STACK_SIZE_PARAM_IS_A_RESERVATION 0x00010000
#endif
#if !defined(PRO_TASK_STACK_SIZE)
#define PRO_TASK_STACK_SIZE               (1024 * 1024 * 1)
#endif

#else

#if !defined(PRO_TASK_STACK_SIZE)
#define PRO_TASK_STACK_SIZE               (1024 * 1024 * 1)
#endif

#endif

/////////////////////////////////////////////////////////////////////////////
////

XUThreadBase::XUThreadBase()
{
    m_threadCount = 0;
    m_realtime    = false;
}

XUThreadBase::~XUThreadBase()
{

}

bool
XUThreadBase::spawn(bool realtime)
{
    {
        XUMutexGuard mon(m_lock);
        
#if defined(_WIN32_WCE)
        
        const HANDLE threadHandle = ::CreateThread(NULL, PRO_TASK_STACK_SIZE,
            &XUThreadBase::svcRun, this, CREATE_SUSPENDED | STACK_SIZE_PARAM_IS_A_RESERVATION, NULL);
        if (threadHandle == NULL)
        {
            return false;
        }
        
        if (realtime)
        {
            ::SetThreadPriority(threadHandle, THREAD_PRIORITY_TIME_CRITICAL);
        }
        
        ::ResumeThread(threadHandle);
        ::CloseHandle(threadHandle);
        
#elif defined(WIN32)
        
        const HANDLE threadHandle = (HANDLE)::_beginthreadex(NULL, PRO_TASK_STACK_SIZE,
            &XUThreadBase::svcRun, this, CREATE_SUSPENDED | STACK_SIZE_PARAM_IS_A_RESERVATION, NULL);
        if (threadHandle == NULL)
        {
            return false;
        }
        
        if (realtime)
        {
            ::SetThreadPriority(threadHandle, THREAD_PRIORITY_TIME_CRITICAL);
        }
        
        ::ResumeThread(threadHandle);
        ::CloseHandle(threadHandle);
        
#else
        
        pthread_attr_t* const attr = new pthread_attr_t;
        pthread_attr_init(attr);
        pthread_attr_setstacksize(attr, PRO_TASK_STACK_SIZE);
        
        pthread_t threadId = 0;
        const int ret = pthread_create(&threadId, attr, &XUThreadBase::svcRun, this);
        
        pthread_attr_destroy(attr);
        delete attr;
        
        if (ret == -1)
        {
            return false;
        }
        
#endif
        
        ++m_threadCount;
        m_realtime = realtime;
    }
    
    return true;
}

void
XUThreadBase::wait()
{
    XUMutexGuard mon(m_lock);

    while (m_threadCount > 0)
    {
        m_cond.wait(&m_lock);
    }
}

#if defined(_WIN32_WCE)
unsigned long
__stdcall
XUThreadBase::svcRun(void * arg)
#elif defined(WIN32)
unsigned int
__stdcall
XUThreadBase::svcRun(void * arg)
#else
void *
svcRun::svcRun(void * arg)
#endif
{
    XUThreadBase * const thread = (XUThreadBase *)arg;
    
    {
        XUMutexGuard mon(thread->m_lock);
    }
    
    srand((unsigned int)time(NULL)); //// !!!
    
#if defined(WIN32) || defined(_WIN32_WCE)

#else
    const pthread_t threadId = pthread_self();
    if (thread->m_realtime)
    {
        struct sched_param param;
        param.sched_priority = sched_get_priority_max(SCHED_RR);
        pthread_setschedparam(threadId, SCHED_RR, &param);
    }
#endif
    
    thread->svc();
    
#if defined(WIN32) || defined(_WIN32_WCE)

#endif
    
    {
        XUMutexGuard mon(thread->m_lock);
        
        --thread->m_threadCount;
        thread->m_cond.signal();
    }
    
#if defined(WIN32) || defined(_WIN32_WCE)
    return 0;
#else
    pthread_detach(threadId);
    
    return 0;
#endif
}

/////////////////////////////////////////////////////////////////////////////
////
