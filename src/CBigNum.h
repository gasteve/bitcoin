#ifndef CBIGNUM_H
#define CBIGNUM_H

#include "uint256.h"
#include <stdexcept>
#include <vector>
#include <openssl/bn.h>
#include "CAutoBN_CTX.h" 

class CBigNum : public BIGNUM
{
public:
    CBigNum();
    CBigNum(const CBigNum& b);
    CBigNum& operator=(const CBigNum& b);
    ~CBigNum();
    CBigNum(char n);
    CBigNum(short n);
    CBigNum(int n);
    CBigNum(long n);
    CBigNum(int64 n);
    CBigNum(unsigned char n);
    CBigNum(unsigned short n);
    CBigNum(unsigned int n);
    CBigNum(unsigned long n);
    CBigNum(uint64 n);
    explicit CBigNum(uint256 n);
    explicit CBigNum(const std::vector<unsigned char>& vch);
    void setulong(unsigned long n);
    unsigned long getulong() const;
    unsigned int getuint() const;
    int getint() const;
    void setint64(int64 n);
    void setuint64(uint64 n);
    void setuint256(uint256 n);
    uint256 getuint256();
    void setvch(const std::vector<unsigned char>& vch);
    std::vector<unsigned char> getvch() const;
    CBigNum& SetCompact(unsigned int nCompact);
    unsigned int GetCompact() const;
    void SetHex(const std::string& str);
    std::string ToString(int nBase=10) const;
    std::string GetHex() const;
    unsigned int GetSerializeSize(int nType=0, int nVersion=VERSION) const;
    template<typename Stream> void Serialize(Stream& s, int nType=0, int nVersion=VERSION) const;
    template<typename Stream> void Unserialize(Stream& s, int nType=0, int nVersion=VERSION);
    bool operator!() const;
    CBigNum& operator+=(const CBigNum& b);
    CBigNum& operator-=(const CBigNum& b);
    CBigNum& operator*=(const CBigNum& b);
    CBigNum& operator/=(const CBigNum& b);
    CBigNum& operator%=(const CBigNum& b);
    CBigNum& operator<<=(unsigned int shift);
    CBigNum& operator>>=(unsigned int shift);
    CBigNum& operator++();
    const CBigNum operator++(int);
    CBigNum& operator--();
    const CBigNum operator--(int);
    friend inline const CBigNum operator-(const CBigNum& a, const CBigNum& b);
    friend inline const CBigNum operator/(const CBigNum& a, const CBigNum& b);
    friend inline const CBigNum operator%(const CBigNum& a, const CBigNum& b);
};

class bignum_error : public std::runtime_error
{
public:
    explicit bignum_error(const std::string& str);
};

#include "CBigNum-inl.h"

#endif
