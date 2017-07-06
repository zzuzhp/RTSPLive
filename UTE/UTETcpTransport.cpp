#include "UTETcpTransport.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////

UTETcpTransport::UTETcpTransport(asio::io_service & io) : UTETransport<UTETcpTransport,
                                                                       asio::ip::tcp::socket,
                                                                       asio::ip::tcp::endpoint>(io, UTE_TRANSPORT_TCP)
{

}

UTETcpTransport::~UTETcpTransport()
{

}

const void
UTETcpTransport::set_observer(IUTETransportObserver * observer)
{
    __super::set_observer(observer);
    read();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////
