//
// string stored as a fixed length field
//

#ifndef CFIXEDFIELDSTRING_H
#define CFIXEDFIELDSTRING_H

#include "serialize.h"

template<std::size_t LEN>
class CFixedFieldString
{
protected:
    const string* pcstr;
    string* pstr;
public:
    explicit CFixedFieldString(const string& str);
    explicit CFixedFieldString(string& str);
    unsigned int GetSerializeSize(int, int=0) const;
    template<typename Stream> void Serialize(Stream& s, int, int=0) const;
    template<typename Stream> void Unserialize(Stream& s, int, int=0);
};

#include "CFixedFieldString-inl.h"

#endif
