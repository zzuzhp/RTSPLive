#include "UTE.h"
#include "UTECore.h"

#include <time.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
////

UTEAPI(IUTE *)
UTECreate()
{
    srand((unsigned)time(0));
    return new UTECore();
}

UTEAPI(void)
UTEDestroy(IUTE * ute)
{
    delete ute;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////
