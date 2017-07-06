#include "UTEUdpTransport.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////

UTEUdpTransport::UTEUdpTransport(asio::io_service & io) : UTETransport<UTEUdpTransport,
                                                          asio::ip::udp::socket,
                                                          asio::ip::udp::endpoint>(io, UTE_TRANSPORT_UDP)
{

}

UTEUdpTransport::~UTEUdpTransport()
{

}

bool
UTEUdpTransport::open(std::string   remote_ip, 
                      uint16_t      remote_port, 
                      std::string   local_ip, 
                      uint16_t      local_port)
{
    if (!local_ip.empty() || local_port != 0)
    {
        asio::error_code ec;
        asio::ip::udp::endpoint local(local_ip.empty() ? asio::ip::address_v4::any() : asio::ip::address::from_string(local_ip), local_port);

        m_socket->open(asio::ip::udp::v4(), ec);
        m_socket->set_option(asio::socket_base::reuse_address(true));
        m_socket->bind(local, ec);

        if (ec)
        {
            /* port occupied? */
            return false;
        }
    }

    asio::ip::udp::resolver::query query(remote_ip, std::to_string(remote_port));

    m_resolver = ASIO_UResolver(new asio::ip::udp::resolver(m_strand.get_io_service()));
    m_resolver->async_resolve(query, m_strand.wrap(UTE_CALLBACK_2(&UTEUdpTransport::on_resolve, shared_from_this())));

    return true;
}

void 
UTEUdpTransport::on_resolve(const asio::error_code & err, asio::ip::udp::resolver::iterator itr)
{
    /* If the initiating socket is not connection-mode, then connect() sets the socket's peer address, 
       but no connection is made. For SOCK_DGRAM sockets, the peer address identifies where all datagrams 
       are sent on subsequent send() calls, and limits the remote sender for subsequent recv() calls.
     */

    /* do not use 'asio::async_connect', it will change local port */
    m_socket->async_connect(*itr, m_strand.wrap(UTE_CALLBACK_1(&UTEUdpTransport::on_connect, shared_from_this())));
}

void 
UTEUdpTransport::on_connect(const asio::error_code & err)
{
    m_socket->set_option(asio::socket_base::reuse_address(true));

    /* once connected, we can get local and remote endpoints safely */
    m_local_endpoint  = m_socket->local_endpoint();
    m_remote_endpoint = m_socket->remote_endpoint();

    read();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////
