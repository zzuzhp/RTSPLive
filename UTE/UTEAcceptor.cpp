#include "UTEAcceptor.h"
#include "UTETcpTransport.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////

UTEAcceptor::UTEAcceptor(asio::io_service & io) : UTEModule(io),
                                                  m_accept(nullptr)
{

}

UTEAcceptor::~UTEAcceptor()
{
    cancel();
}

void
UTEAcceptor::accept(uint16_t port)
{
    if (m_acceptor)
    {
        m_acceptor.reset();
    }

    m_acceptor = ASIO_Acceptor(new asio::ip::tcp::acceptor(m_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)));

    listen();
}

void
UTEAcceptor::listen()
{
    if (!m_acceptor->is_open())
    {
        return;
    }

    UTE_TTransport transport = UTETcpTransport::create_instance(m_service);

    m_acceptor->async_accept(*transport->m_socket,
                             m_strand.wrap(std::bind(&UTEAcceptor::on_accept,
                                                     shared_from_this(),
                                                     transport,
                                                     std::placeholders::_1)));
}

void
UTEAcceptor::cancel()
{
    if (m_acceptor)
    {
        asio::error_code err;
        m_acceptor->cancel(err);

        m_acceptor->close(err);

        //check_error(err, 0);
    }
}

void
UTEAcceptor::on_accept(UTE_TTransport transport, const asio::error_code & err)
{
#if 0
    if (!check_error(err, 0))
    {
        return;
    }
#endif
    transport->m_local_endpoint  = transport->m_socket->local_endpoint();
    transport->m_remote_endpoint = transport->m_socket->remote_endpoint();

    if (m_accept)
    {
        m_accept(shared_from_this(), transport);
    }

    listen();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////
