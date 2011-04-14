//
// Wrapper for serializing arrays and POD
// There's a clever template way to make arrays serialize normally, but MSVC6 doesn't support it
//

#include "CFlatData.h"

CFlatData::CFlatData(void* pbeginIn, void* pendIn) : pbegin((char*)pbeginIn), pend((char*)pendIn) 
{ 
}

char* CFlatData::begin() 
{ 
    return pbegin; 
}

const char* CFlatData::begin() const 
{ 
    return pbegin; 
}

char* CFlatData::end() 
{ 
    return pend; 
}

const char* CFlatData::end() const 
{ 
    return pend; 
}

unsigned int CFlatData::GetSerializeSize(int, int) const
{
    return pend - pbegin;
}
