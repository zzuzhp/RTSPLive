#ifndef ___UTECORE_H___
#define ___UTECORE_H___

#include "UTE.h"
#include "UTEService.h"
#include "XUtil/XUMutex.h"
#include "XUtil/XUThread.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////

class UTECore : public IUTE,
                public XUThreadBase
{
public:

    UTECore();

    ~UTECore();

    std::shared_ptr<IUTEAcceptor> create_acceptor(IUTEAcceptorObserver *observer);

    std::shared_ptr<IUTEConnector> create_connector(IUTEConnectorObserver *observer);

    std::shared_ptr<IUTETransport> create_transport(IUTETransportObserver *observer,
                                                    std::string            remote_ip,
                                                    uint16_t               remote_port,
                                                    std::string            local_ip,
                                                    uint16_t               local_port);

    bool start();

    void stop();

private:

    void on_accept(UTE_Acceptor acceptor, std::shared_ptr<UTETcpTransport> transport);

    void svc();

    void on_service_started(XUMutexCondition *cond);

private:

    ASIO_Service           m_service;
    IUTEAcceptorObserver * m_acceptor_observer;
    XUMutex                m_lock;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___UTECORE_H___
