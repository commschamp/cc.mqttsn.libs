#pragma once

#include "UnitTestCommonBase.h"

class UnitTestNoGwBase : public UnitTestCommonBase
{
    using Base = UnitTestCommonBase;
protected:

    UnitTestNoGwBase(): 
        Base(getFuncs())
    {
    }

    static const LibFuncs& getFuncs();
};