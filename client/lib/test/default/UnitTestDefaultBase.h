#pragma once

#include "UnitTestCommonBase.h"

class UnitTestDefaultBase : public UnitTestCommonBase
{
    using Base = UnitTestCommonBase;
protected:

    UnitTestDefaultBase():
        Base(getFuncs())
    {
    }

    static const LibFuncs& getFuncs();
};