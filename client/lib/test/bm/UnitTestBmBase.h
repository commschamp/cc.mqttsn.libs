#pragma once

#include "UnitTestCommonBase.h"

class UnitTestBmBase : public UnitTestCommonBase
{
    using Base = UnitTestCommonBase;
protected:

    UnitTestBmBase(): 
        Base(getFuncs())
    {
    }

    static const LibFuncs& getFuncs();
};