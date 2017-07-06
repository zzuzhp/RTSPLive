#ifndef ___UTECLIENT_H___
#define ___UTECLIENT_H___

#include "UTEModule.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////

class IUTEClient
{
public:
    virtual void connect(const std::string host, uint16_t port) = 0;
};

template <typename module>
class UTEClient : public IUTEClient,
                  public UTEModule<module>
{
    UTE_BASE(UTEClient)
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___UTECLIENT_H___
