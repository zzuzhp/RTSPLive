#include "XUMutex.h"

#if defined(WIN32) || defined(_WIN32_WCE)
#include <windows.h>
#else
#include <pthread.h>
#endif

/////////////////////////////////////////////////////////////////////////////
////

#if defined(WIN32) || defined(_WIN32_WCE)

class XUMutexImpl
{
public:
    
    XUMutexImpl()
    {
        ::InitializeCriticalSection(&m_cs);
    }
    
    virtual ~XUMutexImpl()
    {
        ::DeleteCriticalSection(&m_cs);
    }
    
    void lock()
    {
        ::EnterCriticalSection(&m_cs);
    }
    
    void unlock()
    {
        ::LeaveCriticalSection(&m_cs);
    }
    
private:
    
    CRITICAL_SECTION m_cs;
};

class XUMutexConditionImpl
{
public:
    
    XUMutexConditionImpl()
    {
        m_sem = ::CreateSemaphore(NULL, 0, 1, NULL);
    }
    
    virtual ~XUMutexConditionImpl()
    {
        if (m_sem != NULL)
        {
            ::CloseHandle(m_sem);
            m_sem = NULL;
        }
    }
    
    void wait(XUMutex * mutex)
    {
        if (mutex != NULL)
        {
            mutex->unlock();
        }
        
        ::WaitForSingleObject(m_sem, INFINITE);
        
        if (mutex != NULL)
        {
            mutex->lock();
        }
    }
    
    void waitr(XURecursiveMutex * rmutex)
    {
        if (rmutex != NULL)
        {
            rmutex->unlock();
        }
        
        ::WaitForSingleObject(m_sem, INFINITE);
        
        if (rmutex != NULL)
        {
            rmutex->lock();
        }
    }
    
    void signal()
    {
        ::ReleaseSemaphore(m_sem, 1, NULL);
    }
    
private:
    
    HANDLE m_sem;
};

#else

class XUMutexImpl
{
    friend class XUMutexConditionImpl;
    
public:
    
    XUMutexImpl()
    {
        pthread_mutex_init(&m_mutext, NULL);
    }
    
    virtual ~XUMutexImpl()
    {
        pthread_mutex_destroy(&m_mutext);
    }
    
    void lock()
    {
        pthread_mutex_lock(&m_mutext);
    }
    
    void unlock()
    {
        pthread_mutex_unlock(&m_mutext);
    }
    
private:
    
    pthread_mutex_t m_mutext;
};

class XUMutexConditionImpl
{
public:
    
    XUMutexConditionImpl()
    {
        m_signal  = false;
        m_waiters = 0;
        pthread_cond_init(&m_condt, NULL);
    }
    
    virtual ~XUMutexConditionImpl()
    {
        pthread_cond_destroy(&m_condt);
    }
    
    void wait(XUMutex * mutex)
    {
        if (mutex != NULL)
        {
            mutex->unlock();
        }
        
        m_mutex.lock();

        while (!m_signal)
        {
            ++m_waiters;
            pthread_cond_wait(&m_condt, &m_mutex.m_mutext);
            --m_waiters;
        }
        
        m_signal = false;
        
        m_mutex.unlock();
        
        if (mutex != NULL)
        {
            mutex->lock();
        }
    }
    
    void waitr(XURecursiveMutex * rmutex)
    {
        if (rmutex != NULL)
        {
            rmutex->unlock();
        }
        
        wait(NULL);
        
        if (rmutex != NULL)
        {
            rmutex->lock();
        }
    }
    
    void signal()
    {
        m_mutex.Lock();
        
        m_signal = true;
        
        if (m_waiters > 0)
        {
            pthread_cond_signal(&m_condt);
        }
        
        m_mutex.unlock();
    }
    
private:
    
    bool            m_signal;
    unsigned long   m_waiters;
    pthread_cond_t  m_condt;
    XUMutexImpl     m_mutex;
};

#endif

/////////////////////////////////////////////////////////////////////////////
////

XUMutex::XUMutex()
{
    m_impl = new XUMutexImpl;
}

XUMutex::~XUMutex()
{
    delete m_impl;
    m_impl = NULL;
}

void
XUMutex::lock()
{
    m_impl->lock();
}

void
XUMutex::unlock()
{
    m_impl->unlock();
}

/////////////////////////////////////////////////////////////////////////////
////

XURecursiveMutex::XURecursiveMutex()
{
    m_ownerNestingLevel = 0;
    m_ownerThreadId     = 0;
    m_waiters           = 0;
    m_cond              = new XUMutexCondition;
    m_mutex             = new XUMutex;
}

XURecursiveMutex::~XURecursiveMutex()
{
    delete m_cond;
    delete m_mutex;
    m_cond  = NULL;
    m_mutex = NULL;
}

void
XURecursiveMutex::lock()
{
#if defined(WIN32) || defined(_WIN32_WCE)
    const unsigned long threadId = ::GetCurrentThreadId();
#else
    const unsigned long threadId = (unsigned long)pthread_self();
#endif

    m_mutex->lock();

    while (m_ownerNestingLevel > 0 && m_ownerThreadId != threadId)
    {
        ++m_waiters;
        m_cond->wait(m_mutex);
        --m_waiters;
    }

    ++m_ownerNestingLevel;
    m_ownerThreadId = threadId;

    m_mutex->unlock();
}

void
XURecursiveMutex::unlock()
{
#if defined(WIN32) || defined(_WIN32_WCE)
    const unsigned long threadId = ::GetCurrentThreadId();
#else
    const unsigned long threadId = (unsigned long)pthread_self();
#endif
    
    m_mutex->lock();

    if (m_ownerNestingLevel > 0 && m_ownerThreadId == threadId)
    {
        --m_ownerNestingLevel;
        
        if (m_ownerNestingLevel == 0)
        {
            m_ownerThreadId = 0;
            
            if (m_waiters > 0)
            {
                m_cond->signal();
            }
        }
    }
    
    m_mutex->unlock();
}

/////////////////////////////////////////////////////////////////////////////
////

XUMutexGuard::XUMutexGuard(XUMutex & mutex)
{
    m_mutex  = &mutex;
    m_rmutex = NULL;
    m_mutex->lock();
}

XUMutexGuard::XUMutexGuard(XURecursiveMutex & rmutex)
{
    m_mutex  = NULL;
    m_rmutex = &rmutex;
    m_rmutex->lock();
}

XUMutexGuard::~XUMutexGuard()
{
    if (m_mutex != NULL)
    {
        m_mutex->unlock();
    }

    if (m_rmutex != NULL)
    {
        m_rmutex->unlock();
    }

    m_mutex  = NULL;
    m_rmutex = NULL;
}

/////////////////////////////////////////////////////////////////////////////
////

XUMutexCondition::XUMutexCondition()
{
    m_impl = new XUMutexConditionImpl;
}

XUMutexCondition::~XUMutexCondition()
{
    delete m_impl;
    m_impl = NULL;
}

void
XUMutexCondition::wait(XUMutex * mutex)
{
    m_impl->wait(mutex);
}

void
XUMutexCondition::waitr(XURecursiveMutex * rmutex)
{
    m_impl->waitr(rmutex);
}

void
XUMutexCondition::signal()
{
    m_impl->signal();
}

/////////////////////////////////////////////////////////////////////////////
////
