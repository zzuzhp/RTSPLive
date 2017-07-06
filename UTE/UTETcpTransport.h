#ifndef ___UTETCPTRANSPORT_H___
#define ___UTETCPTRANSPORT_H___

#include "UTETransport.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////

class UTETcpTransport : public UTETransport<UTETcpTransport, 
                                            asio::ip::tcp::socket, 
                                            asio::ip::tcp::endpoint>
{
    UTE_OBJECT(UTETcpTransport)

    friend class UTEAcceptor;

public:

    void read(size_t size = 0)
    {
        if (size == 0)
        {
            asio::async_read(*m_socket,
                             m_response,
                             asio::transfer_at_least(1),
                             m_strand.wrap(UTE_CALLBACK_2(&UTETcpTransport::on_read, shared_from_this())));

            m_socket->set_option(asio::socket_base::reuse_address(true));
        }
        else
        {
            m_socket->async_read_some(m_response.prepare(size),
                                      m_strand.wrap(UTE_CALLBACK_2(&UTETcpTransport::on_read, shared_from_this())));
        }
    }

    void write(const char * data, size_t size)
    {
        std::ostream stream(&m_request);
        stream.write(data, size);

        asio::async_write(*m_socket,
                          m_request,
                          m_strand.wrap(UTE_CALLBACK_2(&UTETcpTransport::on_write, shared_from_this())));

        m_request.consume(m_request.size());
    }

    const void set_observer(IUTETransportObserver * observer);

private:

    void close()
    {
        if (m_socket && m_socket->is_open())
        {
            asio::error_code err;
            m_socket->close(err);
#if 0
            if (!check_error(err, 0))
            {
                return;
            }
#endif
        }
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___UTETCPTRANSPORT_H___
