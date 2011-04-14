#ifndef CADDRDB_H
#define CADDRDB_H

#include "headers.h"
#include "CDB.h"

class CAddress;

class CAddrDB : public CDB
{
public:
    CAddrDB(const char* pszMode="r+");
private:
    CAddrDB(const CAddrDB&);
    void operator=(const CAddrDB&);
public:
    bool WriteAddress(const CAddress& addr);
    bool EraseAddress(const CAddress& addr);
    bool LoadAddresses();
};

#endif
