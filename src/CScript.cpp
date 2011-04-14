#include "CScript.h"
#include "CBigNum.h"
#include "script.h"
#include "util.h"
#include "base58.h"

CScript::CScript() { }
CScript::CScript(const CScript& b) : vector<unsigned char>(b.begin(), b.end()) { }
CScript::CScript(const_iterator pbegin, const_iterator pend) : vector<unsigned char>(pbegin, pend) { }
#ifndef _MSC_VER
CScript::CScript(const unsigned char* pbegin, const unsigned char* pend) : vector<unsigned char>(pbegin, pend) { }
#endif

CScript& CScript::push_int64(int64 n)
{
    if (n == -1 || (n >= 1 && n <= 16))
    {
        push_back(n + (OP_1 - 1));
    }
    else
    {
        CBigNum bn(n);
        *this << bn.getvch();
    }
    return *this;
}

CScript& CScript::push_uint64(uint64 n)
{
    if (n >= 1 && n <= 16)
    {
        push_back(n + (OP_1 - 1));
    }
    else
    {
        CBigNum bn(n);
        *this << bn.getvch();
    }
    return *this;
}


CScript& CScript::operator+=(const CScript& b)
{
    insert(end(), b.begin(), b.end());
    return *this;
}

CScript operator+(const CScript& a, const CScript& b)
{
    CScript ret = a;
    ret += b;
    return ret;
}


CScript::CScript(char b)           { operator<<(b); }
CScript::CScript(short b)          { operator<<(b); }
CScript::CScript(int b)            { operator<<(b); }
CScript::CScript(long b)           { operator<<(b); }
CScript::CScript(int64 b)          { operator<<(b); }
CScript::CScript(unsigned char b)  { operator<<(b); }
CScript::CScript(unsigned int b)   { operator<<(b); }
CScript::CScript(unsigned short b) { operator<<(b); }
CScript::CScript(unsigned long b)  { operator<<(b); }
CScript::CScript(uint64 b)         { operator<<(b); }

CScript::CScript(opcodetype b)     { operator<<(b); }
CScript::CScript(const uint256& b) { operator<<(b); }
CScript::CScript(const CBigNum& b) { operator<<(b); }
CScript::CScript(const vector<unsigned char>& b) { operator<<(b); }


CScript& CScript::operator<<(char b)           { return push_int64(b); }
CScript& CScript::operator<<(short b)          { return push_int64(b); }
CScript& CScript::operator<<(int b)            { return push_int64(b); }
CScript& CScript::operator<<(long b)           { return push_int64(b); }
CScript& CScript::operator<<(int64 b)          { return push_int64(b); }
CScript& CScript::operator<<(unsigned char b)  { return push_uint64(b); }
CScript& CScript::operator<<(unsigned int b)   { return push_uint64(b); }
CScript& CScript::operator<<(unsigned short b) { return push_uint64(b); }
CScript& CScript::operator<<(unsigned long b)  { return push_uint64(b); }
CScript& CScript::operator<<(uint64 b)         { return push_uint64(b); }

CScript& CScript::operator<<(opcodetype opcode)
{
    if (opcode < 0 || opcode > 0xff)
        throw runtime_error("CScript::operator<<() : invalid opcode");
    insert(end(), (unsigned char)opcode);
    return *this;
}

CScript& CScript::operator<<(const uint160& b)
{
    insert(end(), sizeof(b));
    insert(end(), (unsigned char*)&b, (unsigned char*)&b + sizeof(b));
    return *this;
}

CScript& CScript::operator<<(const uint256& b)
{
    insert(end(), sizeof(b));
    insert(end(), (unsigned char*)&b, (unsigned char*)&b + sizeof(b));
    return *this;
}

CScript& CScript::operator<<(const CBigNum& b)
{
    *this << b.getvch();
    return *this;
}

CScript& CScript::operator<<(const vector<unsigned char>& b)
{
    if (b.size() < OP_PUSHDATA1)
    {
        insert(end(), (unsigned char)b.size());
    }
    else if (b.size() <= 0xff)
    {
        insert(end(), OP_PUSHDATA1);
        insert(end(), (unsigned char)b.size());
    }
    else if (b.size() <= 0xffff)
    {
        insert(end(), OP_PUSHDATA2);
        unsigned short nSize = b.size();
        insert(end(), (unsigned char*)&nSize, (unsigned char*)&nSize + sizeof(nSize));
    }
    else
    {
        insert(end(), OP_PUSHDATA4);
        unsigned int nSize = b.size();
        insert(end(), (unsigned char*)&nSize, (unsigned char*)&nSize + sizeof(nSize));
    }
    insert(end(), b.begin(), b.end());
    return *this;
}

CScript& CScript::operator<<(const CScript& b)
{
    // I'm not sure if this should push the script or concatenate scripts.
    // If there's ever a use for pushing a script onto a script, delete this member fn
    assert(("warning: pushing a CScript onto a CScript with << is probably not intended, use + to concatenate", false));
    return *this;
}


bool CScript::GetOp(iterator& pc, opcodetype& opcodeRet, vector<unsigned char>& vchRet)
{
     // Wrapper so it can be called with either iterator or const_iterator
     const_iterator pc2 = pc;
     bool fRet = GetOp2(pc2, opcodeRet, &vchRet);
     pc = begin() + (pc2 - begin());
     return fRet;
}

bool CScript::GetOp(iterator& pc, opcodetype& opcodeRet)
{
     const_iterator pc2 = pc;
     bool fRet = GetOp2(pc2, opcodeRet, NULL);
     pc = begin() + (pc2 - begin());
     return fRet;
}

bool CScript::GetOp(const_iterator& pc, opcodetype& opcodeRet, vector<unsigned char>& vchRet) const
{
    return GetOp2(pc, opcodeRet, &vchRet);
}

bool CScript::GetOp(const_iterator& pc, opcodetype& opcodeRet) const
{
    return GetOp2(pc, opcodeRet, NULL);
}

bool CScript::GetOp2(const_iterator& pc, opcodetype& opcodeRet, vector<unsigned char>* pvchRet) const
{
    opcodeRet = OP_INVALIDOPCODE;
    if (pvchRet)
        pvchRet->clear();
    if (pc >= end())
        return false;

    // Read instruction
    if (end() - pc < 1)
        return false;
    unsigned int opcode = *pc++;

    // Immediate operand
    if (opcode <= OP_PUSHDATA4)
    {
        unsigned int nSize;
        if (opcode < OP_PUSHDATA1)
        {
            nSize = opcode;
        }
        else if (opcode == OP_PUSHDATA1)
        {
            if (end() - pc < 1)
                return false;
            nSize = *pc++;
        }
        else if (opcode == OP_PUSHDATA2)
        {
            if (end() - pc < 2)
                return false;
            nSize = 0;
            memcpy(&nSize, &pc[0], 2);
            pc += 2;
        }
        else if (opcode == OP_PUSHDATA4)
        {
            if (end() - pc < 4)
                return false;
            memcpy(&nSize, &pc[0], 4);
            pc += 4;
        }
        if (end() - pc < nSize)
            return false;
        if (pvchRet)
            pvchRet->assign(pc, pc + nSize);
        pc += nSize;
    }

    opcodeRet = (opcodetype)opcode;
    return true;
}


void CScript::FindAndDelete(const CScript& b)
{
    if (b.empty())
        return;
    iterator pc = begin();
    opcodetype opcode;
    do
    {
        while (end() - pc >= b.size() && memcmp(&pc[0], &b[0], b.size()) == 0)
            erase(pc, pc + b.size());
    }
    while (GetOp(pc, opcode));
}


int CScript::GetSigOpCount() const
{
    int n = 0;
    const_iterator pc = begin();
    while (pc < end())
    {
        opcodetype opcode;
        if (!GetOp(pc, opcode))
            break;
        if (opcode == OP_CHECKSIG || opcode == OP_CHECKSIGVERIFY)
            n++;
        else if (opcode == OP_CHECKMULTISIG || opcode == OP_CHECKMULTISIGVERIFY)
            n += 20;
    }
    return n;
}

bool CScript::IsPushOnly() const
{
    if (size() > 200)
        return false;
    const_iterator pc = begin();
    while (pc < end())
    {
        opcodetype opcode;
        if (!GetOp(pc, opcode))
            return false;
        if (opcode > OP_16)
            return false;
    }
    return true;
}

uint160 CScript::GetBitcoinAddressHash160() const
{
    opcodetype opcode;
    vector<unsigned char> vch;
    CScript::const_iterator pc = begin();
    if (!GetOp(pc, opcode, vch) || opcode != OP_DUP) return 0;
    if (!GetOp(pc, opcode, vch) || opcode != OP_HASH160) return 0;
    if (!GetOp(pc, opcode, vch) || vch.size() != sizeof(uint160)) return 0;
    uint160 hash160 = uint160(vch);
    if (!GetOp(pc, opcode, vch) || opcode != OP_EQUALVERIFY) return 0;
    if (!GetOp(pc, opcode, vch) || opcode != OP_CHECKSIG) return 0;
    if (pc != end()) return 0;
    return hash160;
}

string CScript::GetBitcoinAddress() const
{
    uint160 hash160 = GetBitcoinAddressHash160();
    if (hash160 == 0)
        return "";
    return Hash160ToAddress(hash160);
}

void CScript::SetBitcoinAddress(const uint160& hash160)
{
    this->clear();
    *this << OP_DUP << OP_HASH160 << hash160 << OP_EQUALVERIFY << OP_CHECKSIG;
}

void CScript::SetBitcoinAddress(const vector<unsigned char>& vchPubKey)
{
    SetBitcoinAddress(Hash160(vchPubKey));
}

bool CScript::SetBitcoinAddress(const string& strAddress)
{
    this->clear();
    uint160 hash160;
    if (!AddressToHash160(strAddress, hash160))
        return false;
    SetBitcoinAddress(hash160);
    return true;
}


void CScript::PrintHex() const
{
    printf("CScript(%s)\n", HexStr(begin(), end(), true).c_str());
}

string CScript::ToString() const
{
    string str;
    opcodetype opcode;
    vector<unsigned char> vch;
    const_iterator pc = begin();
    while (pc < end())
    {
        if (!str.empty())
            str += " ";
        if (!GetOp(pc, opcode, vch))
        {
            str += "[error]";
            return str;
        }
        if (0 <= opcode && opcode <= OP_PUSHDATA4)
            str += ValueString(vch);
        else
            str += GetOpName(opcode);
    }
    return str;
}

void CScript::print() const
{
    printf("%s\n", ToString().c_str());
}
