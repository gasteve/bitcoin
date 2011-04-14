//
// Double ended buffer combining vector and stream-like interfaces.
// >> and << read and write unformatted data using the above serialization templates.
// Fills with data in linear time; some stringstream implementations take N^2 time.
//

#include "CDataStream.h"
#include "serialize.h"

CDataStream::CDataStream(int nTypeIn, int nVersionIn)
{
    Init(nTypeIn, nVersionIn);
}

CDataStream::CDataStream(const_iterator pbegin, const_iterator pend, int nTypeIn, int nVersionIn) : vch(pbegin, pend)
{
    Init(nTypeIn, nVersionIn);
}

#if !defined(_MSC_VER) || _MSC_VER >= 1300
CDataStream::CDataStream(const char* pbegin, const char* pend, int nTypeIn, int nVersionIn) : vch(pbegin, pend)
{
    Init(nTypeIn, nVersionIn);
}
#endif

CDataStream::CDataStream(const vector_type& vchIn, int nTypeIn, int nVersionIn) : vch(vchIn.begin(), vchIn.end())
{
    Init(nTypeIn, nVersionIn);
}

CDataStream::CDataStream(const vector<char>& vchIn, int nTypeIn, int nVersionIn) : vch(vchIn.begin(), vchIn.end())
{
    Init(nTypeIn, nVersionIn);
}

CDataStream::CDataStream(const vector<unsigned char>& vchIn, int nTypeIn, int nVersionIn) : vch((char*)&vchIn.begin()[0], (char*)&vchIn.end()[0])
{
    Init(nTypeIn, nVersionIn);
}

void CDataStream::Init(int nTypeIn, int nVersionIn)
{
    nReadPos = 0;
    nType = nTypeIn;
    nVersion = nVersionIn;
    state = 0;
    exceptmask = ios::badbit | ios::failbit;
}

CDataStream& CDataStream::operator+=(const CDataStream& b)
{
    vch.insert(vch.end(), b.begin(), b.end());
    return *this;
}

CDataStream operator+(const CDataStream& a, const CDataStream& b)
{
    CDataStream ret = a;
    ret += b;
    return (ret);
}

string CDataStream::str() const
{
    return (string(begin(), end()));
}

//
// Vector subset
//
CDataStream::const_iterator CDataStream::begin() const                     { return vch.begin() + nReadPos; }
CDataStream::iterator CDataStream::begin()                                 { return vch.begin() + nReadPos; }
CDataStream::const_iterator CDataStream::end() const                       { return vch.end(); }
CDataStream::iterator CDataStream::end()                                   { return vch.end(); }
CDataStream::size_type CDataStream::size() const                           { return vch.size() - nReadPos; }
bool CDataStream::empty() const                               { return vch.size() == nReadPos; }
void CDataStream::resize(size_type n, value_type c)         { vch.resize(n + nReadPos, c); }
void CDataStream::reserve(size_type n)                        { vch.reserve(n + nReadPos); }
CDataStream::const_reference CDataStream::operator[](size_type pos) const  { return vch[pos + nReadPos]; }
CDataStream::reference CDataStream::operator[](size_type pos)              { return vch[pos + nReadPos]; }
void CDataStream::clear()                                     { vch.clear(); nReadPos = 0; }
CDataStream::iterator CDataStream::insert(iterator it, const char& x) { return vch.insert(it, x); }
void CDataStream::insert(iterator it, size_type n, const char& x) { vch.insert(it, n, x); }

void CDataStream::insert(iterator it, const_iterator first, const_iterator last)
{
    if (it == vch.begin() + nReadPos && last - first <= nReadPos)
    {
        // special case for inserting at the front when there's room
        nReadPos -= (last - first);
        memcpy(&vch[nReadPos], &first[0], last - first);
    }
    else
        vch.insert(it, first, last);
}

void CDataStream::insert(iterator it, vector<char>::const_iterator first, vector<char>::const_iterator last)
{
    if (it == vch.begin() + nReadPos && last - first <= nReadPos)
    {
        // special case for inserting at the front when there's room
        nReadPos -= (last - first);
        memcpy(&vch[nReadPos], &first[0], last - first);
    }
    else
        vch.insert(it, first, last);
}

#if !defined(_MSC_VER) || _MSC_VER >= 1300
void CDataStream::insert(iterator it, const char* first, const char* last)
{
    if (it == vch.begin() + nReadPos && last - first <= nReadPos)
    {
        // special case for inserting at the front when there's room
        nReadPos -= (last - first);
        memcpy(&vch[nReadPos], &first[0], last - first);
    }
    else
        vch.insert(it, first, last);
}
#endif

CDataStream::iterator CDataStream::erase(iterator it)
{
    if (it == vch.begin() + nReadPos)
    {
        // special case for erasing from the front
        if (++nReadPos >= vch.size())
        {
            // whenever we reach the end, we take the opportunity to clear the buffer
            nReadPos = 0;
            return vch.erase(vch.begin(), vch.end());
        }
        return vch.begin() + nReadPos;
    }
    else
        return vch.erase(it);
}

CDataStream::iterator CDataStream::erase(iterator first, iterator last)
{
    if (first == vch.begin() + nReadPos)
    {
        // special case for erasing from the front
        if (last == vch.end())
        {
            nReadPos = 0;
            return vch.erase(vch.begin(), vch.end());
        }
        else
        {
            nReadPos = (last - vch.begin());
            return last;
        }
    }
    else
        return vch.erase(first, last);
}

bool CDataStream::Rewind(size_type n)
{
    // Rewind by n characters if the buffer hasn't been compacted yet
    if (n > nReadPos)
        return false;
    nReadPos -= n;
    return true;
}

//
// Stream subset
//
void CDataStream::setstate(short bits, const char* psz)
{
    state |= bits;
    if (state & exceptmask)
        throw std::ios_base::failure(psz);
}

bool CDataStream::eof() const             { return size() == 0; }
bool CDataStream::fail() const            { return state & (ios::badbit | ios::failbit); }
bool CDataStream::good() const            { return !eof() && (state == 0); }
void CDataStream::clear(short n)          { state = n; }  // name conflict with vector clear()
short CDataStream::exceptions()           { return exceptmask; }
short CDataStream::exceptions(short mask) { short prev = exceptmask; exceptmask = mask; setstate(0, "CDataStream"); return prev; }
CDataStream* CDataStream::rdbuf()         { return this; }
int CDataStream::in_avail()               { return size(); }

void CDataStream::SetType(int n)          { nType = n; }
int CDataStream::GetType()                { return nType; }
void CDataStream::SetVersion(int n)       { nVersion = n; }
int CDataStream::GetVersion()             { return nVersion; }
void CDataStream::ReadVersion()           { *this >> nVersion; }
void CDataStream::WriteVersion()          { *this << nVersion; }

CDataStream& CDataStream::read(char* pch, int nSize)
{
    // Read from the beginning of the buffer
    assert(nSize >= 0);
    unsigned int nReadPosNext = nReadPos + nSize;
    if (nReadPosNext >= vch.size())
    {
        if (nReadPosNext > vch.size())
        {
            setstate(ios::failbit, "CDataStream::read() : end of data");
            memset(pch, 0, nSize);
            nSize = vch.size() - nReadPos;
        }
        memcpy(pch, &vch[nReadPos], nSize);
        nReadPos = 0;
        vch.clear();
        return (*this);
    }
    memcpy(pch, &vch[nReadPos], nSize);
    nReadPos = nReadPosNext;
    return (*this);
}

CDataStream& CDataStream::ignore(int nSize)
{
    // Ignore from the beginning of the buffer
    assert(nSize >= 0);
    unsigned int nReadPosNext = nReadPos + nSize;
    if (nReadPosNext >= vch.size())
    {
        if (nReadPosNext > vch.size())
        {
            setstate(ios::failbit, "CDataStream::ignore() : end of data");
            nSize = vch.size() - nReadPos;
        }
        nReadPos = 0;
        vch.clear();
        return (*this);
    }
    nReadPos = nReadPosNext;
    return (*this);
}

CDataStream& CDataStream::write(const char* pch, int nSize)
{
    // Write to the end of the buffer
    assert(nSize >= 0);
    vch.insert(vch.end(), pch, pch + nSize);
    return (*this);
}
