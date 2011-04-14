#ifndef CDB_H
#define CDB_H

#include "headers.h"

class CDataStream;

class CDB
{
protected:
    Db* pdb;
    string strFile;
    vector<DbTxn*> vTxn;
    bool fReadOnly;

    explicit CDB(const char* pszFile, const char* pszMode="r+");
    ~CDB();
public:
    void Close();
private:
    CDB(const CDB&);
    void operator=(const CDB&);

protected:
    template<typename K, typename T> bool Read(const K& key, T& value);
    template<typename K, typename T> bool Write(const K& key, const T& value, bool fOverwrite=true);
    template<typename K> bool Erase(const K& key);
    template<typename K> bool Exists(const K& key);

    Dbc* GetCursor();
    int ReadAtCursor(Dbc* pcursor, CDataStream& ssKey, CDataStream& ssValue, unsigned int fFlags=DB_NEXT);
    DbTxn* GetTxn();

public:
    bool TxnBegin();
    bool TxnCommit();
    bool TxnAbort();
    bool ReadVersion(int& nVersion);
    bool WriteVersion(int nVersion);
};

#include "CDB-inl.h"

#endif
