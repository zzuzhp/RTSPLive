#ifndef ___XUFUNCTORCOMMANDTASK_H___
#define ___XUFUNCTORCOMMANDTASK_H___

#include "XUThread.h"
#include "XUMutex.h"

#if defined(_MSC_VER)
#pragma warning(disable : 4786)
#endif

#include <list>
#include <set>
#include <vector>

/////////////////////////////////////////////////////////////////////////////
////

class IXUFunctorCommand;

/////////////////////////////////////////////////////////////////////////////
////

class XUFunctorCommandTask : public XUThreadBase
{
public:
    
    XUFunctorCommandTask();
    
    virtual ~XUFunctorCommandTask();
    
    bool start(bool realtime = false, unsigned long threadCount = 1);
    
    void stop();
    
    bool put(IXUFunctorCommand * command);
    
    unsigned long size() const;
    
    const std::vector<unsigned long> threadIds() const;
    
private:
    
    void stopMe();
    
    virtual void svc();
    
private:
    
    unsigned long                  m_threadCountSum;
    unsigned long                  m_threadCountNow;
    bool                           m_wantExit;
    std::set<unsigned long>        m_threadIds;
    std::list<IXUFunctorCommand *> m_commands;
    XUMutexCondition               m_countCond;
    XUMutexCondition               m_commandCond;
    mutable XUMutex                m_lock;
    XUMutex                        m_lockInit;
};

/////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___XUFUNCTORCOMMANDTASK_H___
