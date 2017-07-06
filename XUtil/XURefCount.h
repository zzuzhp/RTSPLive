#ifndef ___XUREFCOUNT_H___
#define ___XUREFCOUNT_H___

#include "XUMutex.h"

/////////////////////////////////////////////////////////////////////////////
////

class XURefCount
{
public:
    
    virtual unsigned long addRef();
    
    virtual unsigned long release();
    
protected:
    
    XURefCount()
    {
        m_refCount = 1;
    }
    
    virtual ~XURefCount()
    {
    }
    
private:
    
    unsigned long   m_refCount;
    
#if !defined(WIN32) && !defined(_WIN32_WCE)
    XUMutex m_lock;
#endif
};

/////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___XUREFCOUNT_H___
