#pragma once

#include "UnitTestCommonBase.h"

class UnitTestQos0Base : public UnitTestCommonBase
{
    using Base = UnitTestCommonBase;
protected:

    UnitTestQos0Base(): 
        Base(getFuncs())
    {
    }

    static const LibFuncs& getFuncs();
};