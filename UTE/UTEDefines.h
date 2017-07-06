#ifndef ___UTEDEFINES_H___
#define ___UTEDEFINES_H___

#include <string>
#include <functional>
#include "asio/asio.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////
//// Unified Transport Entry

/*!
 * typedefs
 */

class UTETcpTransport;
class UTEAcceptor;

/* ute typedefs */
typedef std::shared_ptr<UTEAcceptor>                UTE_Acceptor;
typedef std::shared_ptr<UTETcpTransport>            UTE_TTransport;

/* asio typedefs */
typedef std::shared_ptr<asio::ip::udp::resolver>    ASIO_UResolver;
typedef std::shared_ptr<asio::ip::tcp::resolver>    ASIO_TResolver;
typedef std::shared_ptr<asio::ip::tcp::acceptor>    ASIO_Acceptor;
typedef std::shared_ptr<asio::ip::udp::socket>      ASIO_USocket;
typedef std::shared_ptr<asio::ip::tcp::socket>      ASIO_TSocket;
typedef std::shared_ptr<asio::io_service>           ASIO_Service;

/*!
 * callbacks
 */

typedef std::function<void (std::string, size_t)>   UTE_F_Error;
typedef std::function<void (char *, size_t)>        UTE_F_Read;
typedef std::function<void (size_t, size_t)>        UTE_F_Read_Complete;
typedef std::function<void (size_t)>                UTE_F_Write;
typedef std::function<void (UTE_Acceptor, UTE_TTransport)>     UTE_F_Tcp_Accept;
typedef std::function<void ()>                      UTE_F_Close;
typedef std::function<void ()>                      UTE_F_Cancel;

/*!
 * macros
 */

#define UTE_CALLBACK_0(func, obj, ...) std::bind(func, obj, ##__VA_ARGS__)
#define UTE_CALLBACK_1(func, obj, ...) std::bind(func, obj, std::placeholders::_1, ##__VA_ARGS__)
#define UTE_CALLBACK_2(func, obj, ...) std::bind(func, obj, std::placeholders::_1, std::placeholders::_2, ##__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___UTEDEFINES_H___
