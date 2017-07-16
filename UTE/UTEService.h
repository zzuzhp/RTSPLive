#ifndef ___UTESERVICE_H___
#define ___UTESERVICE_H___

#include "asio/io_service.hpp"
#include "UTEDefines.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////

class UTEService
{
public:

    virtual ~UTEService() {}

    template <typename T0, typename T1>
    void register_error_handler(T0 func, T1 obj)
    { 
        m_f_error = UTE_CALLBACK_2(func, obj);
    }

protected:

    UTEService(asio::io_service &io) : m_f_error(nullptr),
                                       m_service(io),
                                       m_strand(io)
    {

    }

protected:

    asio::io_service          & m_service;
    asio::io_service::strand    m_strand;
    UTE_F_Error                 m_f_error;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___NETWORKS_DISPATCHER_H___
