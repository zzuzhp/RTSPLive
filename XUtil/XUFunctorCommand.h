#ifndef ___XUFUNCTORCOMMAND_H___
#define ___XUFUNCTORCOMMAND_H___

#include <cassert>
#include <cstdint>

/////////////////////////////////////////////////////////////////////////////
////

class IXUFunctorCommand
{
public:
    
    virtual void destroy() = 0;
    
    virtual void execute() = 0;
};

/////////////////////////////////////////////////////////////////////////////
////

template<typename RECEIVER, typename ACTION, unsigned long argCount = 9>
class XUFunctorCommand_cpp : public IXUFunctorCommand
{
public:
    
    static XUFunctorCommand_cpp * createInstance(RECEIVER & receiver,
                                                 ACTION     action,
                                                 int64_t    arg1 = 0,
                                                 int64_t    arg2 = 0,
                                                 int64_t    arg3 = 0,
                                                 int64_t    arg4 = 0,
                                                 int64_t    arg5 = 0,
                                                 int64_t    arg6 = 0,
                                                 int64_t    arg7 = 0,
                                                 int64_t    arg8 = 0,
                                                 int64_t    arg9 = 0)
    {
        assert(action != NULL);
        assert(argCount <= 9);
        if (action == NULL || argCount > 9)
        {
            return NULL;
        }
        
        XUFunctorCommand_cpp * const command = 
            new XUFunctorCommand_cpp(receiver,
                                     action,
                                     arg1,
                                     arg2,
                                     arg3,
                                     arg4,
                                     arg5,
                                     arg6,
                                     arg7,
                                     arg8,
                                     arg9);
        
        return command;
    }
    
private:
    
    XUFunctorCommand_cpp(RECEIVER & receiver,
                         ACTION     action,
                         int64_t    arg1,
                         int64_t    arg2,
                         int64_t    arg3,
                         int64_t    arg4,
                         int64_t    arg5,
                         int64_t    arg6,
                         int64_t    arg7,
                         int64_t    arg8,
                         int64_t    arg9) : m_receiver(receiver)
    {
        m_action = action;
        
        const int64_t args[9] = {arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9};
        memcpy(m_args, args, sizeof(m_args));
    }
    
    virtual ~XUFunctorCommand_cpp()
    {
    }
    
    virtual void destroy()
    {
        delete this;
    }
    
    virtual void execute()
    {
        if (argCount == 0)
        {
            (m_receiver.*m_action)(NULL);
        }
        else
        {
            (m_receiver.*m_action)((int64_t *)m_args);
        }
    }
    
private:
    
    RECEIVER & m_receiver;
    ACTION     m_action;
    int64_t    m_args[argCount + 1];
};

/////////////////////////////////////////////////////////////////////////////
////

template<typename ACTION, unsigned long argCount = 9>
class XUFunctorCommand_c : public IXUFunctorCommand
{
public:
    
    static XUFunctorCommand_c * createInstance(ACTION  action,
                                               int64_t arg1 = 0,
                                               int64_t arg2 = 0,
                                               int64_t arg3 = 0,
                                               int64_t arg4 = 0,
                                               int64_t arg5 = 0,
                                               int64_t arg6 = 0,
                                               int64_t arg7 = 0,
                                               int64_t arg8 = 0,
                                               int64_t arg9 = 0)
    {
        assert(action != NULL);
        assert(argCount <= 9);
        if (action == NULL || argCount > 9)
        {
            return NULL;
        }
        
        XUFunctorCommand_c * const command = 
            new XUFunctorCommand_c(action,
                                   arg1,
                                   arg2,
                                   arg3,
                                   arg4,
                                   arg5,
                                   arg6,
                                   arg7,
                                   arg8,
                                   arg9);
        
        return command;
    }
    
private:
    
    XUFunctorCommand_c(ACTION  action,
                       int64_t arg1,
                       int64_t arg2,
                       int64_t arg3,
                       int64_t arg4,
                       int64_t arg5,
                       int64_t arg6,
                       int64_t arg7,
                       int64_t arg8,
                       int64_t arg9)
    {
        m_action = action;
        
        const int64_t args[9] = {arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9};
        memcpy(m_args, args, sizeof(m_args));
    }
    
    virtual ~XUFunctorCommand_c()
    {
    }
    
    virtual void destroy()
    {
        delete this;
    }
    
    virtual void execute()
    {
        if (argCount == 0)
        {
            (*m_action)(NULL);
        }
        else
        {
            (*m_action)((int64_t *)m_args);
        }
    }
    
private:
    
    ACTION  m_action;
    int64_t m_args[argCount + 1];
};

/////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___XUFUNCTORCOMMAND_H___
