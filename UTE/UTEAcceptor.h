#ifndef ___UTEACCEPTOR_H___
#define ___UTEACCEPTOR_H___

#include "UTEModule.h"
#include "UTE.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////

class UTEAcceptor : public IUTEAcceptor,
                    public UTEModule<UTEAcceptor>
{
    UTE_OBJECT(UTEAcceptor)

public:

    void accept(uint16_t port);

    void cancel();

    template <typename T1, typename T2>
    inline void register_accept_handler(T1 fun, T2 * obj)
    {
        m_accept = UTE_CALLBACK_2(fun, obj);
    }

private:

    void listen();

    void on_accept(UTE_TTransport transport, const asio::error_code & err);

private:

    ASIO_Acceptor      m_acceptor;
    UTE_F_Tcp_Accept   m_accept;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___UTETCPACCEPTOR_H___
