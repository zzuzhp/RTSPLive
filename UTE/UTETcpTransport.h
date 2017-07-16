#ifndef ___UTETCPTRANSPORT_H___
#define ___UTETCPTRANSPORT_H___

#include "UTETransport.h"
#include "XUtil/XUMutex.h"

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
            /* 'asio::aysnc_read': handler will not be called until specified bytes are read or an error occurs. */

            /* Read the input data. The @c m_read_buf streambuf will
               automatically grow to accommodate the entire data. The growth may be
               limited by passing a maximum size to the streambuf constructor.
             */
            asio::async_read(*m_socket,
                              m_read_buf,
                              asio::transfer_at_least(1),
                              m_strand.wrap(UTE_CALLBACK_2(&UTETcpTransport::on_read, shared_from_this(), false)));

            m_socket->set_option(asio::socket_base::reuse_address(true));
        }
        else
        {
            /* 'socket::aysnc_read_some' will read no more than 'size' bytes. */
            m_socket->async_read_some(m_read_buf.prepare(size),
                                      m_strand.wrap(UTE_CALLBACK_2(&UTETcpTransport::on_read, shared_from_this(), true)));
        }
    }

    void write(const char *data, size_t size)
    {
        XUMutexGuard mon(m_lock);

        m_pending_request->sputn(data, size);

        if (m_active_request->size() == 0) ///< 'size()' returns the size of the input sequence
        {
            /* no active write operation */
            std::swap(m_pending_request, m_active_request);

            /* 'asio::async_write' will ensure that all data is written before the asynchronous operation completes
               as opposed to 'basic_stream_socket::async_write_some', which may not transmit all of the data to the peer.
               http://www.boost.org/doc/libs/1_53_0/doc/html/boost_asio/reference/basic_stream_socket/async_write_some.html
             */
            asio::async_write(*m_socket,
                              *m_active_request,
                               m_strand.wrap(UTE_CALLBACK_2(&UTETcpTransport::on_write, shared_from_this())));
        }

        /* https://sourceforge.net/p/asio/mailman/message/3803421/
           Q: May I send multiple async_write in parallel ? i.e.invoke the second
           async_write before the first one callback invoked ?

           A: Yep, it will work if you use one of the member functions (i.e.
           async_write_some or async_send), although do keep in mind that the
           completion handlers may be executed in a different order to the writes.
           It may not work correctly if you try to do multiple asio::async_write()
           operations in parallel.
         */
    }

    const void set_observer(IUTETransportObserver *observer);

protected:

    virtual void on_read(const asio::error_code &ec, size_t bytes_transferred, bool manual)
    {
        if (!ec)
        {
            __super::on_read(ec, bytes_transferred, manual);
        }
        else if (ec != asio::error::operation_aborted)
        {
            /* client disconnected ? */
        }
    }

    virtual void on_write(const asio::error_code &ec, size_t bytes_transferred)
    {
        if (!ec)
        {
            XUMutexGuard mon(m_lock);

            // TODO: why 'm_active_request->size()' can be non-zero here ???
            if (m_active_request->size() == 0 && m_pending_request->size() > 0)
            {
                std::swap(m_pending_request, m_active_request);

                asio::async_write(*m_socket,
                                  *m_active_request,
                                   m_strand.wrap(UTE_CALLBACK_2(&UTETcpTransport::on_write, shared_from_this())));
            }
        }

        __super::on_write(ec, bytes_transferred);
    }

private:

    void close()
    {
        if (m_socket && m_socket->is_open())
        {
            asio::error_code ec;
            m_socket->close(ec);
        }
    }

private:

    /* the input sequence: use double buffer to avoid interleaving call to 'asio::async_write' */
    std::shared_ptr<asio::streambuf>    m_active_request;
    std::shared_ptr<asio::streambuf>    m_pending_request;
    XUMutex                             m_lock;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___UTETCPTRANSPORT_H___
