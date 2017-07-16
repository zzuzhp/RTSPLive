#ifndef ___UTEMODULE_H___
#define ___UTEMODULE_H___

#include "UTEService.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////

template <typename module>
class UTEModule : public UTEService,
                  public std::enable_shared_from_this<module>
{
public:

    static std::shared_ptr<module> create_instance(asio::io_service &io)
    {
        return std::shared_ptr<module>(new module(io))->shared_from_this();
    }

    virtual ~UTEModule() {};

protected:

    UTEModule(asio::io_service &io) : UTEService(io) {};
};

#define UTE_BASE(module) \
public: \
    virtual ~module(){} \
protected: \
    module(asio::io_service &io) : UTEModule(io){}

#define UTE_OBJECT(module) \
    friend class UTEModule<module>; \
public: \
    virtual ~module(); \
protected: \
    module(asio::io_service &io);

////////////////////////////////////////////////////////////////////////////////////////////////////
////

#endif ///< ___UTEMODULE_H___
