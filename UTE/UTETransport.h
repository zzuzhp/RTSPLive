#ifndef ___UTETRANSPORT_H___
#define ___UTETRANSPORT_H___

#include "UTE.h"
#include "UTEModule.h"
#include <memory>

class IUTESession
{
public:

    virtual void read(size_t size = 0) = 0;

    virtual void write(const char * data, size_t size) = 0;
};

template <typename module, typename SOCK, typename ENDPOINT>
class UTETransport : public IUTETransport,
                     public IUTESession,
                     public UTEModule<module>
{
public:

    virtual ~UTETransport()
    {
        m_response.consume(m_response.size());
        m_request.consume(m_request.size());
    }

    const void set_observer(IUTETransportObserver * observer) { m_observer = observer; }

    const UTE_TRANSPORT_TYPE type() { return m_type; }

    std::string local_ip() { return m_local_endpoint.address().to_string(); }

    const uint16_t local_port() { return m_local_endpoint.port(); }

    /* TODO: for an unconnected udp socket, remote address should be fetched on 'recv_from' */
    std::string remote_ip() { return m_remote_endpoint.address().to_string(); }

    const uint16_t remote_port() { return m_remote_endpoint.port(); }

    void send(const char * data, int len) { write(data, len); }

protected:

    UTETransport(asio::io_service &io, UTE_TRANSPORT_TYPE type) : UTEModule(io),
                                                                  m_observer(nullptr),
                                                                  m_type(type)
    {
        m_local_endpoint  = ENDPOINT(asio::ip::address::from_string("0.0.0.0"), 0);
        m_remote_endpoint = ENDPOINT(asio::ip::address::from_string("0.0.0.0"), 0);

        m_socket = std::shared_ptr<SOCK>(new SOCK(io));
    }

    void on_read(const asio::error_code & err, size_t bytes_transferred)
    {
        if (bytes_transferred > 0)
        {
            char * data = new char[bytes_transferred + 1];
            data[bytes_transferred] = 0;

            m_response.commit(bytes_transferred);
            std::istream stream(&m_response);
            stream.read(data, bytes_transferred);

            if (m_observer)
            {
                m_observer->on_recv(shared_from_this(), data, bytes_transferred);
            }

            read();

            delete[] data;
        }

        ///< check_error(err, bytes_transferred, err != asio::error::eof);

        if ((err == asio::error::eof || (m_buffer_size > 0 && bytes_transferred < m_buffer_size)))
        {
            if (m_observer)
            {
                m_observer->on_message(shared_from_this(), UTE_READ_DONE, "read complete", nullptr);
            }
        }

        m_response.consume(m_response.size());
    }

    void on_write(const asio::error_code & err, size_t bytes_transferred)
    {
#if 0
        if (!check_error(err, bytes_transferred))
        {
            return;
        }
#endif
        if (m_observer)
        {
            m_observer->on_send(shared_from_this(), bytes_transferred);
        }
    }

protected:

    std::shared_ptr<SOCK>     m_socket;
    ENDPOINT                  m_local_endpoint;
    ENDPOINT                  m_remote_endpoint;
    size_t                    m_buffer_size;
    asio::streambuf           m_request;
    asio::streambuf           m_response;

private:

    UTE_TRANSPORT_TYPE        m_type;
    IUTETransportObserver   * m_observer;
};

#endif ///< ___UTETRANSPORT_H___
