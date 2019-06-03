#include "XURefCount.h"
#include "XUMutex.h"

#if defined(_WIN32)
#include <windows.h>
#endif

/////////////////////////////////////////////////////////////////////////////
////

unsigned long
XURefCount::addRef()
{
#if defined(_WIN32)
    const unsigned long refCount = ::InterlockedIncrement((long*)&m_refCount);
#else
    m_lock.lock();
    const unsigned long refCount = ++m_refCount;
    m_lock.unlock();
#endif
    
    return refCount;
}

unsigned long
XURefCount::release()
{
#if defined(_WIN32)
    const unsigned long refCount = ::InterlockedDecrement((long*)&m_refCount);
#else
    m_lock.lock();
    const unsigned long refCount = --m_refCount;
    m_lock.unlock();
#endif
    
    if (refCount == 0)
    {
        delete this;
    }
    
    return refCount;
}

/////////////////////////////////////////////////////////////////////////////
////
