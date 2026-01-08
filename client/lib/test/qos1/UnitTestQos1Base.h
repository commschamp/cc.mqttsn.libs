#pragma once

#include "UnitTestCommonBase.h"

class UnitTestQos1Base : public UnitTestCommonBase
{
    using Base = UnitTestCommonBase;
protected:

    UnitTestQos1Base():
        Base(getFuncs())
    {
    }

    static const LibFuncs& getFuncs();
};