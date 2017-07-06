#include "RTSPLive.h"
#include "RTSPCore.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////

RTSPLIVEAPI(IRTSPLive *)
RTSPLiveCreate(uint16_t listening_port)
{
    RTSPCore * core = new RTSPCore();
    if (!core)
    {
        return nullptr;
    }

    if (!core->build(listening_port))
    {
        delete core;
        return nullptr;
    }

    return core;
}

RTSPLIVEAPI(void)
RTSPLiveDestroy(IRTSPLive * live)
{
    delete live;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////
