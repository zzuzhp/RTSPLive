#ifndef ___XUMUTEX_H___
#define ___XUMUTEX_H___

/////////////////////////////////////////////////////////////////////////////
////

class XUMutexCondition;
class XUMutexConditionImpl;
class XUMutexImpl;

/////////////////////////////////////////////////////////////////////////////
////

class XUMutex
{
public:
    
    XUMutex();
    
    virtual ~XUMutex();
    
    void lock();
    
    void unlock();
    
private:
    
    XUMutexImpl * m_impl;
};

/////////////////////////////////////////////////////////////////////////////
////

class XURecursiveMutex
{
public:
    
    XURecursiveMutex();
    
    virtual ~XURecursiveMutex();
    
    void lock();
    
    void unlock();
    
private:
    
    unsigned long         m_ownerNestingLevel;
    unsigned long         m_ownerThreadId;
    unsigned long         m_waiters;
    XUMutexCondition    * m_cond;
    XUMutex             * m_mutex;
};

/////////////////////////////////////////////////////////////////////////////
////

class XUMutexGuard
{
public:
    
    XUMutexGuard(XUMutex & mutex);
    
    XUMutexGuard(XURecursiveMutex & rmutex);
    
    virtual ~XUMutexGuard();
    
private:
    
    XUMutex             * m_mutex;
    XURecursiveMutex    * m_rmutex;
};

/////////////////////////////////////////////////////////////////////////////
////

class XUMutexCondition
{
public:
    
    XUMutexCondition();
    
    virtual ~XUMutexCondition();
    
    void wait(XUMutex * mutex = 0);
    
    void waitr(XURecursiveMutex * rmutex);
    
    void signal();
    
private:
    
    XUMutexConditionImpl    * m_impl;
};

/////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___XUMUTEX_H___
