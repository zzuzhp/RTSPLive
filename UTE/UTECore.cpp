#include "UTECore.h"
#include "UTEAcceptor.h"
#include "UTETcpTransport.h"
#include "UTEUdpTransport.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////

UTECore::UTECore() : m_acceptor_observer(nullptr)
{
    m_service.reset(new asio::io_service());
}

UTECore::~UTECore()
{
    stop();
}

std::shared_ptr<IUTEAcceptor>
UTECore::create_acceptor(IUTEAcceptorObserver * observer)
{
    UTE_Acceptor acceptor = UTEAcceptor::create_instance(*m_service);

    acceptor->register_accept_handler(&UTECore::on_accept, this);
    m_acceptor_observer = observer;

    return acceptor;
}

std::shared_ptr<IUTEConnector> 
UTECore::create_connector(IUTEConnectorObserver * observer)
{
    return nullptr;
}

std::shared_ptr<IUTETransport> 
UTECore::create_transport(IUTETransportObserver * observer,
                          std::string             remote_ip,
                          uint16_t                remote_port,
                          std::string             local_ip,
                          uint16_t                local_port)
{
    std::shared_ptr<UTEUdpTransport> transport = UTEUdpTransport::create_instance(*m_service);

    if (!transport->open(remote_ip, remote_port, local_ip, local_port))
    {
        return nullptr;
    }

    transport->set_observer(observer);

    return transport;
}

bool
UTECore::start()
{
    return spawn();
}

void
UTECore::stop()
{
    m_service->stop();
    wait();
}

void 
UTECore::on_accept(UTE_Acceptor acceptor, std::shared_ptr<UTETcpTransport> transport)
{
    if (m_acceptor_observer)
    {
        m_acceptor_observer->on_accept(acceptor, transport);
    }
}

void
UTECore::svc()
{
    /* run the service */
    asio::io_service::work work(*m_service);
    m_service->run();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////
