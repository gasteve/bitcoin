//
// Wrapper for serializing arrays and POD
// There's a clever template way to make arrays serialize normally, but MSVC6 doesn't support it
//

#ifndef CFLATDATA_H
#define CFLATDATA_H

#include "serialize.h"

class CFlatData
{
protected:
    char* pbegin;
    char* pend;
public:
    CFlatData(void* pbeginIn, void* pendIn);
    char* begin();
    const char* begin() const;
    char* end();
    const char* end() const;
    unsigned int GetSerializeSize(int, int=0) const;
    template<typename Stream> void Serialize(Stream& s, int, int=0) const;
    template<typename Stream> void Unserialize(Stream& s, int, int=0);
};

#include "CFlatData-inl.h"

#endif
