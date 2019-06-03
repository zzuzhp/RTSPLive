#include "XUFunctorCommandTask.h"
#include "XUFunctorCommand.h"
#include "XUThread.h"
#include "XUMutex.h"

#if defined(_MSC_VER)
#pragma warning(disable : 4786)
#endif

#if defined(_WIN32)
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <cassert>
#include <list>
#include <set>
#include <vector>

/////////////////////////////////////////////////////////////////////////////
////

XUFunctorCommandTask::XUFunctorCommandTask()
{
    m_threadCountSum = 0;
    m_threadCountNow = 0;
    m_wantExit       = false;
}

XUFunctorCommandTask::~XUFunctorCommandTask()
{
    stop();
}

bool
XUFunctorCommandTask::start(bool          realtime,    /* = false */
                            unsigned long threadCount) /* = 1 */
{{
    XUMutexGuard mon(m_lockInit);
    
    assert(threadCount > 0);
    if (threadCount == 0)
    {
        return false;
    }
    
    do
    {
        {
            XUMutexGuard mon(m_lock);
            
            assert(m_threadCountSum == 0);
            if (m_threadCountSum != 0)
            {
                return false;
            }
            
            m_threadCountSum = threadCount; //// Stop()!!!
            
            int i = 0;
            
            for (; i < (int)threadCount; ++i)
            {
                if (!spawn(realtime))
                {
                    break;
                }
            }
            
            assert(i == (int)threadCount);
            if (i != (int)threadCount)
            {
                break;
            }
            
            while (m_threadCountNow < m_threadCountSum)
            {
                m_countCond.wait(&m_lock);
            }
        }
        
        return true;
    }
    while (0);
    
    stopMe();
    
    return false;
}}

void
XUFunctorCommandTask::stop()
{{
    XUMutexGuard mon(m_lockInit);
    
    stopMe();
}}

void
XUFunctorCommandTask::stopMe()
{
#if defined(_WIN32) 
    const unsigned long threadId = ::GetCurrentThreadId();
#else
    const unsigned long threadId = (unsigned long)pthread_self();
#endif
    
    {
        XUMutexGuard mon(m_lock);
        
        if (m_threadCountSum == 0 || m_wantExit) //// m_wantExit!!!
        {
            return;
        }
        
        assert(m_threadIds.find(threadId) == m_threadIds.end()); //// deadlock!!!
        
        m_wantExit = true;
        m_commandCond.signal();
    }
    
    wait();
    
    {
        XUMutexGuard mon(m_lock);
        
        m_threadCountSum = 0;
        m_threadCountNow = 0;
        m_wantExit       = false;
    }
}

bool
XUFunctorCommandTask::put(IXUFunctorCommand * command)
{
    assert(command != NULL);
    if (command == NULL)
    {
        return false;
    }
    
    XUMutexCondition * cond = NULL;
    
    {
        XUMutexGuard mon(m_lock);
        
        if (m_threadCountSum == 0 || m_threadCountNow != m_threadCountSum)
        {
            return false;
        }
        
        if (m_wantExit)
        {
            return false;
        }
        
        m_commands.push_back(command);
        m_commandCond.signal();
    }
    
    return true;
}

unsigned long
XUFunctorCommandTask::size() const
{
    unsigned long size = 0;
    
    {
        XUMutexGuard mon(m_lock);
        
        size = (unsigned long)m_commands.size();
    }
    
    return size;
}

const std::vector<unsigned long>
XUFunctorCommandTask::threadIds() const
{
    std::vector<unsigned long> threadIds;
    
    {
        XUMutexGuard mon(m_lock);
        
        std::set<unsigned long>::const_iterator       itr = m_threadIds.begin();
        std::set<unsigned long>::const_iterator const end = m_threadIds.end();
        
        for (; itr != end; ++itr)
        {
            threadIds.push_back(*itr);
        }
    }
    
    return threadIds;
}

void
XUFunctorCommandTask::svc()
{
#if defined(_WIN32) 
    const unsigned long threadId = ::GetCurrentThreadId();
#else
    const unsigned long threadId = (unsigned long)pthread_self();
#endif
    
    {
        XUMutexGuard mon(m_lock);
        
        ++m_threadCountNow;
        m_threadIds.insert(threadId);
        m_countCond.signal();
    }
    
    std::list<IXUFunctorCommand *> commands;
    
    while (1)
    {
        IXUFunctorCommand * command = NULL;
        
        {
            XUMutexGuard mon(m_lock);
            
            while (1)
            {
                if (m_wantExit || m_commands.size() > 0)
                {
                    break;
                }
                
                m_commandCond.wait(&m_lock);
            }
            
            if (m_wantExit)
            {
                commands = m_commands;
                m_commands.clear();
                break;
            }
            
            command = m_commands.front();
            m_commands.pop_front();
        }
        
        command->execute();
        command->destroy();
    }
    
    while (commands.size() > 0)
    {
        IXUFunctorCommand * const command = commands.front();
        commands.pop_front();
        command->execute();
        command->destroy();
    }
    
    {
        XUMutexGuard mon(m_lock);
        
        m_threadIds.erase(threadId);
    }
}

/////////////////////////////////////////////////////////////////////////////
////
