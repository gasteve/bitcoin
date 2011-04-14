//
// Double ended buffer combining vector and stream-like interfaces.
// >> and << read and write unformatted data using the above serialization templates.
// Fills with data in linear time; some stringstream implementations take N^2 time.
//
#ifndef CDATASTREAM_H
#define CDATASTREAM_H

#include "serialize.h"

class CDataStream
{
protected:
    typedef vector<char, secure_allocator<char> > vector_type;
    vector_type vch;
    unsigned int nReadPos;
    short state;
    short exceptmask;
public:
    int nType;
    int nVersion;

    typedef vector_type::allocator_type   allocator_type;
    typedef vector_type::size_type        size_type;
    typedef vector_type::difference_type  difference_type;
    typedef vector_type::reference        reference;
    typedef vector_type::const_reference  const_reference;
    typedef vector_type::value_type       value_type;
    typedef vector_type::iterator         iterator;
    typedef vector_type::const_iterator   const_iterator;
    typedef vector_type::reverse_iterator reverse_iterator;

    explicit CDataStream(int nTypeIn=SER_NETWORK, int nVersionIn=VERSION);
    CDataStream(const_iterator pbegin, const_iterator pend, int nTypeIn=SER_NETWORK, int nVersionIn=VERSION);

#if !defined(_MSC_VER) || _MSC_VER >= 1300
    CDataStream(const char* pbegin, const char* pend, int nTypeIn=SER_NETWORK, int nVersionIn=VERSION);
#endif

    CDataStream(const vector_type& vchIn, int nTypeIn=SER_NETWORK, int nVersionIn=VERSION);
    CDataStream(const vector<char>& vchIn, int nTypeIn=SER_NETWORK, int nVersionIn=VERSION);
    CDataStream(const vector<unsigned char>& vchIn, int nTypeIn=SER_NETWORK, int nVersionIn=VERSION);
    void Init(int nTypeIn=SER_NETWORK, int nVersionIn=VERSION);
    CDataStream& operator+=(const CDataStream& b);
    friend CDataStream operator+(const CDataStream& a, const CDataStream& b);
    string str() const;

    //
    // Vector subset
    //
    const_iterator begin() const;
    iterator begin();
    const_iterator end() const;
    iterator end();
    size_type size() const;
    bool empty() const;
    void resize(size_type n, value_type c=0);
    void reserve(size_type n);
    const_reference operator[](size_type pos) const;
    reference operator[](size_type pos);
    void clear();
    iterator insert(iterator it, const char& x=char());
    void insert(iterator it, size_type n, const char& x);

    void insert(iterator it, const_iterator first, const_iterator last);
    void insert(iterator it, vector<char>::const_iterator first, vector<char>::const_iterator last);

#if !defined(_MSC_VER) || _MSC_VER >= 1300
    void insert(iterator it, const char* first, const char* last);
#endif

    iterator erase(iterator it);
    iterator erase(iterator first, iterator last);

    inline void Compact();
    bool Rewind(size_type n);

    //
    // Stream subset
    //
    void setstate(short bits, const char* psz);
    bool eof() const;
    bool fail() const;
    bool good() const;
    void clear(short n); // name conflict with vector clear()
    short exceptions();
    short exceptions(short mask);
    CDataStream* rdbuf();
    int in_avail();

    void SetType(int n);
    int GetType();
    void SetVersion(int n);
    int GetVersion();
    void ReadVersion();
    void WriteVersion();

    CDataStream& read(char* pch, int nSize);
    CDataStream& ignore(int nSize);
    CDataStream& write(const char* pch, int nSize);
    
    template<typename Stream> void Serialize(Stream& s, int nType=0, int nVersion=VERSION) const;
    template<typename T> unsigned int GetSerializeSize(const T& obj);
    template<typename T> CDataStream& operator<<(const T& obj);
    template<typename T> CDataStream& operator>>(T& obj);
};

#include "CDataStream-inl.h"

#endif
