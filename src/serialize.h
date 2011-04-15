// Copyright (c) 2009-2010 Satoshi Nakamoto
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.

#ifndef SERIALIZE_H
#define SERIALIZE_H

#include "headers.h"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <boost/type_traits/is_fundamental.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/tuple/tuple_io.hpp>
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef __int64  int64;
typedef unsigned __int64  uint64;
#else
typedef long long  int64;
typedef unsigned long long  uint64;
#endif
#if defined(_MSC_VER) && _MSC_VER < 1300
#define for  if (false) ; else for
#endif
class CScript;
class CDataStream;
class CAutoFile;
static const unsigned int MAX_SIZE = 0x02000000;

static const int VERSION = 32100;
static const char* pszSubVer = "";
static const bool VERSION_IS_BETA = true;

/////////////////////////////////////////////////////////////////
//
// Templates for serializing to anything that looks like a stream,
// i.e. anything that supports .read(char*, int) and .write(char*, int)
//

enum
{
    // primary actions
    SER_NETWORK         = (1 << 0),
    SER_DISK            = (1 << 1),
    SER_GETHASH         = (1 << 2),

    // modifiers
    SER_SKIPSIG         = (1 << 16),
    SER_BLOCKHEADERONLY = (1 << 17),
};

#define IMPLEMENT_SERIALIZE(statements)    \
    unsigned int GetSerializeSize(int nType=0, int nVersion=VERSION) const  \
    {                                           \
        CSerActionGetSerializeSize ser_action;  \
        const bool fGetSize = true;             \
        const bool fWrite = false;              \
        const bool fRead = false;               \
        unsigned int nSerSize = 0;              \
        ser_streamplaceholder s;                \
        s.nType = nType;                        \
        s.nVersion = nVersion;                  \
        {statements}                            \
        return nSerSize;                        \
    }                                           \
    template<typename Stream>                   \
    void Serialize(Stream& s, int nType=0, int nVersion=VERSION) const  \
    {                                           \
        CSerActionSerialize ser_action;         \
        const bool fGetSize = false;            \
        const bool fWrite = true;               \
        const bool fRead = false;               \
        unsigned int nSerSize = 0;              \
        {statements}                            \
    }                                           \
    template<typename Stream>                   \
    void Unserialize(Stream& s, int nType=0, int nVersion=VERSION)  \
    {                                           \
        CSerActionUnserialize ser_action;       \
        const bool fGetSize = false;            \
        const bool fWrite = false;              \
        const bool fRead = true;                \
        unsigned int nSerSize = 0;              \
        {statements}                            \
    }

#define READWRITE(obj)      (nSerSize += ::SerReadWrite(s, (obj), nType, nVersion, ser_action))

//
// Basic types
//
#define WRITEDATA(s, obj)   s.write((char*)&(obj), sizeof(obj))
#define READDATA(s, obj)    s.read((char*)&(obj), sizeof(obj))

inline unsigned int GetSerializeSize(char a,           int, int=0) { return sizeof(a); }
inline unsigned int GetSerializeSize(signed char a,    int, int=0) { return sizeof(a); }
inline unsigned int GetSerializeSize(unsigned char a,  int, int=0) { return sizeof(a); }
inline unsigned int GetSerializeSize(signed short a,   int, int=0) { return sizeof(a); }
inline unsigned int GetSerializeSize(unsigned short a, int, int=0) { return sizeof(a); }
inline unsigned int GetSerializeSize(signed int a,     int, int=0) { return sizeof(a); }
inline unsigned int GetSerializeSize(unsigned int a,   int, int=0) { return sizeof(a); }
inline unsigned int GetSerializeSize(signed long a,    int, int=0) { return sizeof(a); }
inline unsigned int GetSerializeSize(unsigned long a,  int, int=0) { return sizeof(a); }
inline unsigned int GetSerializeSize(int64 a,          int, int=0) { return sizeof(a); }
inline unsigned int GetSerializeSize(uint64 a,         int, int=0) { return sizeof(a); }
inline unsigned int GetSerializeSize(float a,          int, int=0) { return sizeof(a); }
inline unsigned int GetSerializeSize(double a,         int, int=0) { return sizeof(a); }

template<typename Stream> inline void Serialize(Stream& s, char a,           int, int=0) { WRITEDATA(s, a); }
template<typename Stream> inline void Serialize(Stream& s, signed char a,    int, int=0) { WRITEDATA(s, a); }
template<typename Stream> inline void Serialize(Stream& s, unsigned char a,  int, int=0) { WRITEDATA(s, a); }
template<typename Stream> inline void Serialize(Stream& s, signed short a,   int, int=0) { WRITEDATA(s, a); }
template<typename Stream> inline void Serialize(Stream& s, unsigned short a, int, int=0) { WRITEDATA(s, a); }
template<typename Stream> inline void Serialize(Stream& s, signed int a,     int, int=0) { WRITEDATA(s, a); }
template<typename Stream> inline void Serialize(Stream& s, unsigned int a,   int, int=0) { WRITEDATA(s, a); }
template<typename Stream> inline void Serialize(Stream& s, signed long a,    int, int=0) { WRITEDATA(s, a); }
template<typename Stream> inline void Serialize(Stream& s, unsigned long a,  int, int=0) { WRITEDATA(s, a); }
template<typename Stream> inline void Serialize(Stream& s, int64 a,          int, int=0) { WRITEDATA(s, a); }
template<typename Stream> inline void Serialize(Stream& s, uint64 a,         int, int=0) { WRITEDATA(s, a); }
template<typename Stream> inline void Serialize(Stream& s, float a,          int, int=0) { WRITEDATA(s, a); }
template<typename Stream> inline void Serialize(Stream& s, double a,         int, int=0) { WRITEDATA(s, a); }

template<typename Stream> inline void Unserialize(Stream& s, char& a,           int, int=0) { READDATA(s, a); }
template<typename Stream> inline void Unserialize(Stream& s, signed char& a,    int, int=0) { READDATA(s, a); }
template<typename Stream> inline void Unserialize(Stream& s, unsigned char& a,  int, int=0) { READDATA(s, a); }
template<typename Stream> inline void Unserialize(Stream& s, signed short& a,   int, int=0) { READDATA(s, a); }
template<typename Stream> inline void Unserialize(Stream& s, unsigned short& a, int, int=0) { READDATA(s, a); }
template<typename Stream> inline void Unserialize(Stream& s, signed int& a,     int, int=0) { READDATA(s, a); }
template<typename Stream> inline void Unserialize(Stream& s, unsigned int& a,   int, int=0) { READDATA(s, a); }
template<typename Stream> inline void Unserialize(Stream& s, signed long& a,    int, int=0) { READDATA(s, a); }
template<typename Stream> inline void Unserialize(Stream& s, unsigned long& a,  int, int=0) { READDATA(s, a); }
template<typename Stream> inline void Unserialize(Stream& s, int64& a,          int, int=0) { READDATA(s, a); }
template<typename Stream> inline void Unserialize(Stream& s, uint64& a,         int, int=0) { READDATA(s, a); }
template<typename Stream> inline void Unserialize(Stream& s, float& a,          int, int=0) { READDATA(s, a); }
template<typename Stream> inline void Unserialize(Stream& s, double& a,         int, int=0) { READDATA(s, a); }

inline unsigned int GetSerializeSize(bool a, int, int=0)                          { return sizeof(char); }
template<typename Stream> inline void Serialize(Stream& s, bool a, int, int=0)    { char f=a; WRITEDATA(s, f); }
template<typename Stream> inline void Unserialize(Stream& s, bool& a, int, int=0) { char f; READDATA(s, f); a=f; }

//
// Compact size
//  size <  253        -- 1 byte
//  size <= USHRT_MAX  -- 3 bytes  (253 + 2 bytes)
//  size <= UINT_MAX   -- 5 bytes  (254 + 4 bytes)
//  size >  UINT_MAX   -- 9 bytes  (255 + 8 bytes)
//
inline unsigned int GetSizeOfCompactSize(uint64 nSize)
{
    if (nSize < 253)             return sizeof(unsigned char);
    else if (nSize <= USHRT_MAX) return sizeof(unsigned char) + sizeof(unsigned short);
    else if (nSize <= UINT_MAX)  return sizeof(unsigned char) + sizeof(unsigned int);
    else                         return sizeof(unsigned char) + sizeof(uint64);
}

template<typename Stream>
void WriteCompactSize(Stream& os, uint64 nSize)
{
    if (nSize < 253)
    {
        unsigned char chSize = nSize;
        WRITEDATA(os, chSize);
    }
    else if (nSize <= USHRT_MAX)
    {
        unsigned char chSize = 253;
        unsigned short xSize = nSize;
        WRITEDATA(os, chSize);
        WRITEDATA(os, xSize);
    }
    else if (nSize <= UINT_MAX)
    {
        unsigned char chSize = 254;
        unsigned int xSize = nSize;
        WRITEDATA(os, chSize);
        WRITEDATA(os, xSize);
    }
    else
    {
        unsigned char chSize = 255;
        uint64 xSize = nSize;
        WRITEDATA(os, chSize);
        WRITEDATA(os, xSize);
    }
    return;
}

template<typename Stream>
uint64 ReadCompactSize(Stream& is)
{
    unsigned char chSize;
    READDATA(is, chSize);
    uint64 nSizeRet = 0;
    if (chSize < 253)
    {
        nSizeRet = chSize;
    }
    else if (chSize == 253)
    {
        unsigned short xSize;
        READDATA(is, xSize);
        nSizeRet = xSize;
    }
    else if (chSize == 254)
    {
        unsigned int xSize;
        READDATA(is, xSize);
        nSizeRet = xSize;
    }
    else
    {
        uint64 xSize;
        READDATA(is, xSize);
        nSizeRet = xSize;
    }
    if (nSizeRet > (uint64)MAX_SIZE)
        throw std::ios_base::failure("ReadCompactSize() : size too large");
    return nSizeRet;
}

#define FLATDATA(obj)   REF(CFlatData((char*)&(obj), (char*)&(obj) + sizeof(obj)))

//
// Forward declarations
//

// string
template<typename C> unsigned int GetSerializeSize(const basic_string<C>& str, int, int=0);
template<typename Stream, typename C> void Serialize(Stream& os, const basic_string<C>& str, int, int=0);
template<typename Stream, typename C> void Unserialize(Stream& is, basic_string<C>& str, int, int=0);

// vector
template<typename T, typename A> unsigned int GetSerializeSize_impl(const std::vector<T, A>& v, int nType, int nVersion, const boost::true_type&);
template<typename T, typename A> unsigned int GetSerializeSize_impl(const std::vector<T, A>& v, int nType, int nVersion, const boost::false_type&);
template<typename T, typename A> inline unsigned int GetSerializeSize(const std::vector<T, A>& v, int nType, int nVersion=VERSION);
template<typename Stream, typename T, typename A> void Serialize_impl(Stream& os, const std::vector<T, A>& v, int nType, int nVersion, const boost::true_type&);
template<typename Stream, typename T, typename A> void Serialize_impl(Stream& os, const std::vector<T, A>& v, int nType, int nVersion, const boost::false_type&);
template<typename Stream, typename T, typename A> inline void Serialize(Stream& os, const std::vector<T, A>& v, int nType, int nVersion=VERSION);
template<typename Stream, typename T, typename A> void Unserialize_impl(Stream& is, std::vector<T, A>& v, int nType, int nVersion, const boost::true_type&);
template<typename Stream, typename T, typename A> void Unserialize_impl(Stream& is, std::vector<T, A>& v, int nType, int nVersion, const boost::false_type&);
template<typename Stream, typename T, typename A> inline void Unserialize(Stream& is, std::vector<T, A>& v, int nType, int nVersion=VERSION);

// others derived from vector
extern inline unsigned int GetSerializeSize(const CScript& v, int nType, int nVersion=VERSION);
template<typename Stream> void Serialize(Stream& os, const CScript& v, int nType, int nVersion=VERSION);
template<typename Stream> void Unserialize(Stream& is, CScript& v, int nType, int nVersion=VERSION);

// pair
template<typename K, typename T> unsigned int GetSerializeSize(const std::pair<K, T>& item, int nType, int nVersion=VERSION);
template<typename Stream, typename K, typename T> void Serialize(Stream& os, const std::pair<K, T>& item, int nType, int nVersion=VERSION);
template<typename Stream, typename K, typename T> void Unserialize(Stream& is, std::pair<K, T>& item, int nType, int nVersion=VERSION);

// 3 tuple
template<typename T0, typename T1, typename T2> unsigned int GetSerializeSize(const boost::tuple<T0, T1, T2>& item, int nType, int nVersion=VERSION);
template<typename Stream, typename T0, typename T1, typename T2> void Serialize(Stream& os, const boost::tuple<T0, T1, T2>& item, int nType, int nVersion=VERSION);
template<typename Stream, typename T0, typename T1, typename T2> void Unserialize(Stream& is, boost::tuple<T0, T1, T2>& item, int nType, int nVersion=VERSION);

// 4 tuple
template<typename T0, typename T1, typename T2, typename T3> unsigned int GetSerializeSize(const boost::tuple<T0, T1, T2, T3>& item, int nType, int nVersion=VERSION);
template<typename Stream, typename T0, typename T1, typename T2, typename T3> void Serialize(Stream& os, const boost::tuple<T0, T1, T2, T3>& item, int nType, int nVersion=VERSION);
template<typename Stream, typename T0, typename T1, typename T2, typename T3> void Unserialize(Stream& is, boost::tuple<T0, T1, T2, T3>& item, int nType, int nVersion=VERSION);

// map
template<typename K, typename T, typename Pred, typename A> unsigned int GetSerializeSize(const std::map<K, T, Pred, A>& m, int nType, int nVersion=VERSION);
template<typename Stream, typename K, typename T, typename Pred, typename A> void Serialize(Stream& os, const std::map<K, T, Pred, A>& m, int nType, int nVersion=VERSION);
template<typename Stream, typename K, typename T, typename Pred, typename A> void Unserialize(Stream& is, std::map<K, T, Pred, A>& m, int nType, int nVersion=VERSION);

// set
template<typename K, typename Pred, typename A> unsigned int GetSerializeSize(const std::set<K, Pred, A>& m, int nType, int nVersion=VERSION);
template<typename Stream, typename K, typename Pred, typename A> void Serialize(Stream& os, const std::set<K, Pred, A>& m, int nType, int nVersion=VERSION);
template<typename Stream, typename K, typename Pred, typename A> void Unserialize(Stream& is, std::set<K, Pred, A>& m, int nType, int nVersion=VERSION);

//
// If none of the specialized versions above matched, default to calling member function.
// "int nType" is changed to "long nType" to keep from getting an ambiguous overload error.
// The compiler will only cast int to long if none of the other templates matched.
// Thanks to Boost serialization for this idea.
//
template<typename T>
inline unsigned int GetSerializeSize(const T& a, long nType, int nVersion=VERSION)
{
    return a.GetSerializeSize((int)nType, nVersion);
}

template<typename Stream, typename T>
inline void Serialize(Stream& os, const T& a, long nType, int nVersion=VERSION)
{
    a.Serialize(os, (int)nType, nVersion);
}

template<typename Stream, typename T>
inline void Unserialize(Stream& is, T& a, long nType, int nVersion=VERSION)
{
    a.Unserialize(is, (int)nType, nVersion);
}

//
// string
//
template<typename C>
unsigned int GetSerializeSize(const basic_string<C>& str, int, int)
{
    return GetSizeOfCompactSize(str.size()) + str.size() * sizeof(str[0]);
}

template<typename Stream, typename C>
void Serialize(Stream& os, const basic_string<C>& str, int, int)
{
    WriteCompactSize(os, str.size());
    if (!str.empty())
        os.write((char*)&str[0], str.size() * sizeof(str[0]));
}

template<typename Stream, typename C>
void Unserialize(Stream& is, basic_string<C>& str, int, int)
{
    unsigned int nSize = ReadCompactSize(is);
    str.resize(nSize);
    if (nSize != 0)
        is.read((char*)&str[0], nSize * sizeof(str[0]));
}

//
// vector
//
template<typename T, typename A>
unsigned int GetSerializeSize_impl(const std::vector<T, A>& v, int nType, int nVersion, const boost::true_type&)
{
    return (GetSizeOfCompactSize(v.size()) + v.size() * sizeof(T));
}

template<typename T, typename A>
unsigned int GetSerializeSize_impl(const std::vector<T, A>& v, int nType, int nVersion, const boost::false_type&)
{
    unsigned int nSize = GetSizeOfCompactSize(v.size());
    for (typename std::vector<T, A>::const_iterator vi = v.begin(); vi != v.end(); ++vi)
        nSize += GetSerializeSize((*vi), nType, nVersion);
    return nSize;
}

template<typename T, typename A>
inline unsigned int GetSerializeSize(const std::vector<T, A>& v, int nType, int nVersion)
{
    return GetSerializeSize_impl(v, nType, nVersion, boost::is_fundamental<T>());
}

template<typename Stream, typename T, typename A>
void Serialize_impl(Stream& os, const std::vector<T, A>& v, int nType, int nVersion, const boost::true_type&)
{
    WriteCompactSize(os, v.size());
    if (!v.empty())
        os.write((char*)&v[0], v.size() * sizeof(T));
}

template<typename Stream, typename T, typename A>
void Serialize_impl(Stream& os, const std::vector<T, A>& v, int nType, int nVersion, const boost::false_type&)
{
    WriteCompactSize(os, v.size());
    for (typename std::vector<T, A>::const_iterator vi = v.begin(); vi != v.end(); ++vi)
        ::Serialize(os, (*vi), nType, nVersion);
}

template<typename Stream, typename T, typename A>
inline void Serialize(Stream& os, const std::vector<T, A>& v, int nType, int nVersion)
{
    Serialize_impl(os, v, nType, nVersion, boost::is_fundamental<T>());
}


template<typename Stream, typename T, typename A>
void Unserialize_impl(Stream& is, std::vector<T, A>& v, int nType, int nVersion, const boost::true_type&)
{
    //unsigned int nSize = ReadCompactSize(is);
    //v.resize(nSize);
    //is.read((char*)&v[0], nSize * sizeof(T));

    // Limit size per read so bogus size value won't cause out of memory
    v.clear();
    unsigned int nSize = ReadCompactSize(is);
    unsigned int i = 0;
    while (i < nSize)
    {
        unsigned int blk = min(nSize - i, (unsigned int)(1 + 4999999 / sizeof(T)));
        v.resize(i + blk);
        is.read((char*)&v[i], blk * sizeof(T));
        i += blk;
    }
}

template<typename Stream, typename T, typename A>
void Unserialize_impl(Stream& is, std::vector<T, A>& v, int nType, int nVersion, const boost::false_type&)
{
    //unsigned int nSize = ReadCompactSize(is);
    //v.resize(nSize);
    //for (std::vector<T, A>::iterator vi = v.begin(); vi != v.end(); ++vi)
    //    Unserialize(is, (*vi), nType, nVersion);

    v.clear();
    unsigned int nSize = ReadCompactSize(is);
    unsigned int i = 0;
    unsigned int nMid = 0;
    while (nMid < nSize)
    {
        nMid += 5000000 / sizeof(T);
        if (nMid > nSize)
            nMid = nSize;
        v.resize(nMid);
        for (; i < nMid; i++)
            Unserialize(is, v[i], nType, nVersion);
    }
}

template<typename Stream, typename T, typename A>
inline void Unserialize(Stream& is, std::vector<T, A>& v, int nType, int nVersion)
{
    Unserialize_impl(is, v, nType, nVersion, boost::is_fundamental<T>());
}

//
// others derived from vector
//
inline unsigned int GetSerializeSize(const CScript& v, int nType, int nVersion)
{
    return GetSerializeSize((const vector<unsigned char>&)v, nType, nVersion);
}

template<typename Stream>
void Serialize(Stream& os, const CScript& v, int nType, int nVersion)
{
    Serialize(os, (const vector<unsigned char>&)v, nType, nVersion);
}

template<typename Stream>
void Unserialize(Stream& is, CScript& v, int nType, int nVersion)
{
    Unserialize(is, (vector<unsigned char>&)v, nType, nVersion);
}

//
// pair
//
template<typename K, typename T>
unsigned int GetSerializeSize(const std::pair<K, T>& item, int nType, int nVersion)
{
    return GetSerializeSize(item.first, nType, nVersion) + GetSerializeSize(item.second, nType, nVersion);
}

template<typename Stream, typename K, typename T>
void Serialize(Stream& os, const std::pair<K, T>& item, int nType, int nVersion)
{
    Serialize(os, item.first, nType, nVersion);
    Serialize(os, item.second, nType, nVersion);
}

template<typename Stream, typename K, typename T>
void Unserialize(Stream& is, std::pair<K, T>& item, int nType, int nVersion)
{
    Unserialize(is, item.first, nType, nVersion);
    Unserialize(is, item.second, nType, nVersion);
}

//
// 3 tuple
//
template<typename T0, typename T1, typename T2>
unsigned int GetSerializeSize(const boost::tuple<T0, T1, T2>& item, int nType, int nVersion)
{
    unsigned int nSize = 0;
    nSize += GetSerializeSize(get<0>(item), nType, nVersion);
    nSize += GetSerializeSize(get<1>(item), nType, nVersion);
    nSize += GetSerializeSize(get<2>(item), nType, nVersion);
    return nSize;
}

template<typename Stream, typename T0, typename T1, typename T2>
void Serialize(Stream& os, const boost::tuple<T0, T1, T2>& item, int nType, int nVersion)
{
    Serialize(os, get<0>(item), nType, nVersion);
    Serialize(os, get<1>(item), nType, nVersion);
    Serialize(os, get<2>(item), nType, nVersion);
}

template<typename Stream, typename T0, typename T1, typename T2>
void Unserialize(Stream& is, boost::tuple<T0, T1, T2>& item, int nType, int nVersion)
{
    Unserialize(is, get<0>(item), nType, nVersion);
    Unserialize(is, get<1>(item), nType, nVersion);
    Unserialize(is, get<2>(item), nType, nVersion);
}

//
// 4 tuple
//
template<typename T0, typename T1, typename T2, typename T3>
unsigned int GetSerializeSize(const boost::tuple<T0, T1, T2, T3>& item, int nType, int nVersion)
{
    unsigned int nSize = 0;
    nSize += GetSerializeSize(get<0>(item), nType, nVersion);
    nSize += GetSerializeSize(get<1>(item), nType, nVersion);
    nSize += GetSerializeSize(get<2>(item), nType, nVersion);
    nSize += GetSerializeSize(get<3>(item), nType, nVersion);
    return nSize;
}

template<typename Stream, typename T0, typename T1, typename T2, typename T3>
void Serialize(Stream& os, const boost::tuple<T0, T1, T2, T3>& item, int nType, int nVersion)
{
    Serialize(os, get<0>(item), nType, nVersion);
    Serialize(os, get<1>(item), nType, nVersion);
    Serialize(os, get<2>(item), nType, nVersion);
    Serialize(os, get<3>(item), nType, nVersion);
}

template<typename Stream, typename T0, typename T1, typename T2, typename T3>
void Unserialize(Stream& is, boost::tuple<T0, T1, T2, T3>& item, int nType, int nVersion)
{
    Unserialize(is, get<0>(item), nType, nVersion);
    Unserialize(is, get<1>(item), nType, nVersion);
    Unserialize(is, get<2>(item), nType, nVersion);
    Unserialize(is, get<3>(item), nType, nVersion);
}

//
// map
//
template<typename K, typename T, typename Pred, typename A>
unsigned int GetSerializeSize(const std::map<K, T, Pred, A>& m, int nType, int nVersion)
{
    unsigned int nSize = GetSizeOfCompactSize(m.size());
    for (typename std::map<K, T, Pred, A>::const_iterator mi = m.begin(); mi != m.end(); ++mi)
        nSize += GetSerializeSize((*mi), nType, nVersion);
    return nSize;
}

template<typename Stream, typename K, typename T, typename Pred, typename A>
void Serialize(Stream& os, const std::map<K, T, Pred, A>& m, int nType, int nVersion)
{
    WriteCompactSize(os, m.size());
    for (typename std::map<K, T, Pred, A>::const_iterator mi = m.begin(); mi != m.end(); ++mi)
        Serialize(os, (*mi), nType, nVersion);
}

template<typename Stream, typename K, typename T, typename Pred, typename A>
void Unserialize(Stream& is, std::map<K, T, Pred, A>& m, int nType, int nVersion)
{
    m.clear();
    unsigned int nSize = ReadCompactSize(is);
    typename std::map<K, T, Pred, A>::iterator mi = m.begin();
    for (unsigned int i = 0; i < nSize; i++)
    {
        pair<K, T> item;
        Unserialize(is, item, nType, nVersion);
        mi = m.insert(mi, item);
    }
}

//
// set
//
template<typename K, typename Pred, typename A>
unsigned int GetSerializeSize(const std::set<K, Pred, A>& m, int nType, int nVersion)
{
    unsigned int nSize = GetSizeOfCompactSize(m.size());
    for (typename std::set<K, Pred, A>::const_iterator it = m.begin(); it != m.end(); ++it)
        nSize += GetSerializeSize((*it), nType, nVersion);
    return nSize;
}

template<typename Stream, typename K, typename Pred, typename A>
void Serialize(Stream& os, const std::set<K, Pred, A>& m, int nType, int nVersion)
{
    WriteCompactSize(os, m.size());
    for (typename std::set<K, Pred, A>::const_iterator it = m.begin(); it != m.end(); ++it)
        Serialize(os, (*it), nType, nVersion);
}

template<typename Stream, typename K, typename Pred, typename A>
void Unserialize(Stream& is, std::set<K, Pred, A>& m, int nType, int nVersion)
{
    m.clear();
    unsigned int nSize = ReadCompactSize(is);
    typename std::set<K, Pred, A>::iterator it = m.begin();
    for (unsigned int i = 0; i < nSize; i++)
    {
        K key;
        Unserialize(is, key, nType, nVersion);
        it = m.insert(it, key);
    }
}

//
// Support for IMPLEMENT_SERIALIZE and READWRITE macro
//
class CSerActionGetSerializeSize { };
class CSerActionSerialize { };
class CSerActionUnserialize { };

template<typename Stream, typename T>
inline unsigned int SerReadWrite(Stream& s, const T& obj, int nType, int nVersion, CSerActionGetSerializeSize ser_action)
{
    return ::GetSerializeSize(obj, nType, nVersion);
}

template<typename Stream, typename T>
inline unsigned int SerReadWrite(Stream& s, const T& obj, int nType, int nVersion, CSerActionSerialize ser_action)
{
    ::Serialize(s, obj, nType, nVersion);
    return 0;
}

template<typename Stream, typename T>
inline unsigned int SerReadWrite(Stream& s, T& obj, int nType, int nVersion, CSerActionUnserialize ser_action)
{
    ::Unserialize(s, obj, nType, nVersion);
    return 0;
}

struct ser_streamplaceholder
{
    int nType;
    int nVersion;
};

//
// Allocator that clears its contents before deletion
//
template<typename T>
struct secure_allocator : public std::allocator<T>
{
    // MSVC8 default copy constructor is broken
    typedef std::allocator<T> base;
    typedef typename base::size_type size_type;
    typedef typename base::difference_type  difference_type;
    typedef typename base::pointer pointer;
    typedef typename base::const_pointer const_pointer;
    typedef typename base::reference reference;
    typedef typename base::const_reference const_reference;
    typedef typename base::value_type value_type;
    secure_allocator() throw() {}
    secure_allocator(const secure_allocator& a) throw() : base(a) {}
    template <typename U>
    secure_allocator(const secure_allocator<U>& a) throw() : base(a) {}
    ~secure_allocator() throw() {}
    template<typename _Other> struct rebind
    { typedef secure_allocator<_Other> other; };

    void deallocate(T* p, std::size_t n)
    {
        if (p != NULL)
            memset(p, 0, sizeof(T) * n);
        allocator<T>::deallocate(p, n);
    }
};

// Include headers for classes that serialization clients will need so that
// they only need to include serialize.h
#include "CFlatData.h"
#include "CFixedFieldString.h"
#include "CDataStream.h"
#include "CAutoFile.h"

#ifdef TESTCDATASTREAM
// VC6sp6
// CDataStream:
// n=1000       0 seconds
// n=2000       0 seconds
// n=4000       0 seconds
// n=8000       0 seconds
// n=16000      0 seconds
// n=32000      0 seconds
// n=64000      1 seconds
// n=128000     1 seconds
// n=256000     2 seconds
// n=512000     4 seconds
// n=1024000    8 seconds
// n=2048000    16 seconds
// n=4096000    32 seconds
// stringstream:
// n=1000       1 seconds
// n=2000       1 seconds
// n=4000       13 seconds
// n=8000       87 seconds
// n=16000      400 seconds
// n=32000      1660 seconds
// n=64000      6749 seconds
// n=128000     27241 seconds
// n=256000     109804 seconds
#include <iostream>
int main(int argc, char *argv[])
{
    vector<unsigned char> vch(0xcc, 250);
    printf("CDataStream:\n");
    for (int n = 1000; n <= 4500000; n *= 2)
    {
        CDataStream ss;
        time_t nStart = time(NULL);
        for (int i = 0; i < n; i++)
            ss.write((char*)&vch[0], vch.size());
        printf("n=%-10d %d seconds\n", n, time(NULL) - nStart);
    }
    printf("stringstream:\n");
    for (int n = 1000; n <= 4500000; n *= 2)
    {
        stringstream ss;
        time_t nStart = time(NULL);
        for (int i = 0; i < n; i++)
            ss.write((char*)&vch[0], vch.size());
        printf("n=%-10d %d seconds\n", n, time(NULL) - nStart);
    }
}
#endif

#endif
