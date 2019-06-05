#ifndef ___UTE_H___
#define ___UTE_H___

#include <stdint.h>
#include <memory>
#include <string>

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#ifndef UTE_EXTERN_C
#ifdef __cplusplus
#define UTE_EXTERN_C extern "C"
#else
#define UTE_EXTERN_C
#endif
#endif

#if defined(_WIN32)
    #if defined UTE_EXPORTS
        #define UTE_DLLEXPORTS __declspec(dllexport)
    #else
        #define UTE_DLLEXPORTS
    #endif
#else
    #if defined UTE_EXPORTS
        #define UTE_DLLEXPORTS __attribute__ ((visibility ("default")))
    #else
        #define UTE_DLLEXPORTS
    #endif
#endif

#if defined(_WIN32)
#define UTE_CDECL   __cdecl
#else
#define UTE_CDECL
#endif

#ifndef UTEAPI
#define UTEAPI(rettype) UTE_EXTERN_C UTE_DLLEXPORTS rettype UTE_CDECL
#endif

#define UTE_READ_DONE           0x0001
#define UTE_CONN_CLOSE          0x0002
/* error messages */
#define UTE_ERR_RESOLVE         0x8000
#define UTE_ERR_CONNECT         0x8001
#define UTE_ERR_CANCEL          0x8002
#define UTE_ERR_CLOSE           0x8003
#define UTE_ERR_OPEN_SOCKET     0x8004
#define UTE_ERR_BIND_SOCKET     0x8005

typedef uint8_t UTE_TRANSPORT_TYPE;

static const UTE_TRANSPORT_TYPE UTE_TRANSPORT_TCP = 1;
static const UTE_TRANSPORT_TYPE UTE_TRANSPORT_UDP = 2;

class IUTEAcceptor;
class IUTEConnector;
class IUTETransport;

class IUTEAcceptorObserver
{
public:
    virtual void on_accept(std::shared_ptr<IUTEAcceptor> acceptor, std::shared_ptr<IUTETransport> transport) = 0;
};

class IUTEAcceptor
{
public:
    virtual void accept(uint16_t port) = 0;
};

class IUTEConnectorObserver
{
public:
    virtual void on_connect(std::shared_ptr<IUTEConnector> connector, std::shared_ptr<IUTETransport> transport) = 0;
};

class IUTEConnector
{
public:
    virtual void connect(const char * ip, uint16_t port) = 0;
};

class IUTETransportObserver
{
public:
    virtual void on_recv(std::shared_ptr<IUTETransport> transport, const char * data, int len) = 0;

    virtual void on_send(std::shared_ptr<IUTETransport> transport, int len) = 0;

    virtual void on_message(std::shared_ptr<IUTETransport> transport, int code, const char * message, void * arg) = 0;
};

class IUTETransport
{
public:
    virtual const UTE_TRANSPORT_TYPE type() = 0;

    virtual const void set_observer(IUTETransportObserver * observer) = 0;

    virtual std::string local_ip() = 0;

    virtual const uint16_t local_port() = 0;

    virtual std::string remote_ip() = 0;

    virtual const uint16_t remote_port() = 0;

    virtual void send(const char * data, int len) = 0;
};

class IUTE
{
public:
    virtual ~IUTE() {}

    /* tcp */
    virtual std::shared_ptr<IUTEAcceptor> create_acceptor(IUTEAcceptorObserver * observer) = 0;

    virtual std::shared_ptr<IUTEConnector> create_connector(IUTEConnectorObserver * observer) = 0;

    /* udp */
    virtual std::shared_ptr<IUTETransport> create_transport(IUTETransportObserver * observer,
                                                            std::string             remote_ip,
                                                            uint16_t                remote_port,
                                                            std::string             local_ip = "",
                                                            uint16_t                local_port = 0) = 0;

    /* control */
    virtual bool start() = 0;

    virtual void stop() = 0;
};

/****************************************************************************************\
 *                                         APIs                                         *
\****************************************************************************************/

UTEAPI(IUTE *)
UTECreate();

UTEAPI(void)
UTEDestroy(IUTE * ute);

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___UTE_H___
