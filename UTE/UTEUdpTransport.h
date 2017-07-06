#ifndef ___UTEUDPTRANSPORT_H___
#define ___UTEUDPTRANSPORT_H___

#include "UTETransport.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////

class UTEUdpTransport : public UTETransport<UTEUdpTransport,
                                            asio::ip::udp::socket,
                                            asio::ip::udp::endpoint>
{
    UTE_OBJECT(UTEUdpTransport)

public:

    bool open(std::string remote_ip, uint16_t remote_port, std::string local_ip, uint16_t local_port);

    void read(size_t size = 0)
    {
        if (size == 0)
        {
            size = 64 * 1024;
        }

        m_buffer_size = size;
        m_socket->async_receive_from(m_response.prepare(size),
                                     m_remote_endpoint,
                                     m_strand.wrap(UTE_CALLBACK_2(&UTEUdpTransport::on_read, shared_from_this())));
    }

    void write(const char * data, size_t size)
    {
        std::ostream stream(&m_request);
        stream.write(data, size);

        m_socket->async_send(m_request.data(), m_strand.wrap(UTE_CALLBACK_2(&UTEUdpTransport::on_write, shared_from_this())));
        m_request.consume(m_request.size());
    }

private:

    void on_connect(const asio::error_code & err);
    
    void on_resolve(const asio::error_code & err, asio::ip::udp::resolver::iterator itr);

private:

    ASIO_UResolver  m_resolver;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___UTEUDPTRANSPORT_H___
