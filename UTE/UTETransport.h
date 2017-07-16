#ifndef ___UTETRANSPORT_H___
#define ___UTETRANSPORT_H___

#include "UTE.h"
#include "UTEModule.h"

class IUTESession
{
public:

    virtual void read(size_t size = 0) = 0;

    virtual void write(const char *data, size_t size) = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////

template <typename module, typename SOCK, typename ENDPOINT>
class UTETransport : public IUTETransport,
                     public IUTESession,
                     public UTEModule<module>
{
public:

    virtual ~UTETransport()
    {

    }

    const void set_observer(IUTETransportObserver *observer) { m_observer = observer; }

    const UTE_TRANSPORT_TYPE type() { return m_type; }

    std::string local_ip() { return m_local_endpoint.address().to_string(); }

    const uint16_t local_port() { return m_local_endpoint.port(); }

    /* TODO: for an unconnected udp socket, remote address should be fetched on 'recv_from' */
    std::string remote_ip() { return m_remote_endpoint.address().to_string(); }

    const uint16_t remote_port() { return m_remote_endpoint.port(); }

    void send(const char *data, int len) { write(data, len); }

protected:

    UTETransport(asio::io_service &io, UTE_TRANSPORT_TYPE type) : UTEModule(io),
                                                                  m_observer(nullptr),
                                                                  m_type(type)
    {
        m_local_endpoint  = ENDPOINT(asio::ip::address::from_string("0.0.0.0"), 0);
        m_remote_endpoint = ENDPOINT(asio::ip::address::from_string("0.0.0.0"), 0);

        m_socket = std::shared_ptr<SOCK>(new SOCK(io));
    }

    virtual void on_read(const asio::error_code &ec, size_t bytes_transferred, bool is_mutablebuf)
    {
        /* if @c mannal is set, it means a mutable_buffers_type instead of a streambuf
           is passed into the read functions, thus, we need first commit() the bytes
           before we can successfully read from the streambuf.
         */
        if (bytes_transferred > 0)
        {
            if (is_mutablebuf)
            {
                assert(m_read_buf.size() == 0);
                m_read_buf.commit(bytes_transferred);
            }

            /*  Either using a std::istream and reading from it, such as by std::getline(),
                or explicitly invoking asio::streambuf::consume(n), will remove data from the input sequence.
             */
            std::string data(asio::buffers_begin(m_read_buf.data()),
                             asio::buffers_begin(m_read_buf.data()) + bytes_transferred);

            /* If a buffer is provided to an operation instead, such as passing prepare()
               to a read operation or data() to a write operation, then one must explicitly
               handle the commit() and consume().
            */
            m_read_buf.consume(bytes_transferred);
            assert(!is_mutablebuf || m_read_buf.size() == 0);

            if (m_observer)
            {
                m_observer->on_recv(shared_from_this(), data.data(), bytes_transferred);
            }

            read();
        }
    }

    virtual void on_write(const asio::error_code &err, size_t bytes_transferred)
    {
        if (m_observer)
        {
            m_observer->on_send(shared_from_this(), bytes_transferred);
        }
    }

protected:

    std::shared_ptr<SOCK>     m_socket;
    ENDPOINT                  m_local_endpoint;
    ENDPOINT                  m_remote_endpoint;
    asio::streambuf           m_read_buf;

private:

    UTE_TRANSPORT_TYPE        m_type;
    IUTETransportObserver   * m_observer;
};

/* FYI:
   The nomenclature for asio::streambuf is similar to that of which is defined in the C++ standard,
   and used across various classes in the standard template library, wherein data is written to an
   output stream and data is read from an input stream.
   The general lifecycle of data in asio::streambuf is as follows:

   1. buffers get allocated with 'prepare()' for the output sequence.
   2. after data has been written into the output sequence's buffers, the
      data will be 'commit()'ed. this commited data is removed from the
      output sequence and appended to the input sequence from which it can be read.
   3. data is read from the input sequence's buffers obtained via 'data()'.
   4. once data has been read, it can be removed from the input sequence by 'consume()'.
 */
 ////////////////////////////////////////////////////////////////////////////////////////////////////
 ////

#endif ///< ___UTETRANSPORT_H___
