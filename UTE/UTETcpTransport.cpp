#include "UTETcpTransport.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////

UTETcpTransport::UTETcpTransport(asio::io_service &io) : UTETransport<UTETcpTransport,
                                                                      asio::ip::tcp::socket,
                                                                      asio::ip::tcp::endpoint>(io, UTE_TRANSPORT_TCP)
{
    m_active_request  = new asio::streambuf;
    m_pending_request = new asio::streambuf;
}

UTETcpTransport::~UTETcpTransport()
{
    delete m_pending_request;
    delete m_active_request;
}

const void
UTETcpTransport::set_observer(IUTETransportObserver *observer)
{
    __super::set_observer(observer);
    read();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////
