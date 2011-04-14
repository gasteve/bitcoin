#ifndef CKEY_H
#define CKEY_H

#include "headers.h"
#include "serialize.h"
#include "uint256.h"

// secure_allocator is defined in serialize.h
typedef vector<unsigned char, secure_allocator<unsigned char> > CPrivKey;

class CKey
{
protected:
    EC_KEY* pkey;
    bool fSet;

public:
    CKey();
    CKey(const CKey& b);
    CKey& operator=(const CKey& b);
    ~CKey();
    bool IsNull() const;
    void MakeNewKey();
    bool SetPrivKey(const CPrivKey& vchPrivKey);
    CPrivKey GetPrivKey() const;
    bool SetPubKey(const vector<unsigned char>& vchPubKey);
    vector<unsigned char> GetPubKey() const;
    bool Sign(uint256 hash, vector<unsigned char>& vchSig);
    bool Verify(uint256 hash, const vector<unsigned char>& vchSig);
    static bool Sign(const CPrivKey& vchPrivKey, uint256 hash, vector<unsigned char>& vchSig);
    static bool Verify(const vector<unsigned char>& vchPubKey, uint256 hash, const vector<unsigned char>& vchSig);
};

class key_error : public std::runtime_error
{
public:
    explicit key_error(const std::string& str);
};

#endif
