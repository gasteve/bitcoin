#include "CDB.h"
#include "main.h"
#include "CCriticalBlock.h"
#include "db.h"

CDB::~CDB() { Close(); };

Dbc* CDB::GetCursor()
{
    if (!pdb)
        return NULL;
    Dbc* pcursor = NULL;
    int ret = pdb->cursor(NULL, &pcursor, 0);
    if (ret != 0)
        return NULL;
    return pcursor;
}

int CDB::ReadAtCursor(Dbc* pcursor, CDataStream& ssKey, CDataStream& ssValue, unsigned int fFlags)
{
    // Read at cursor
    Dbt datKey;
    if (fFlags == DB_SET || fFlags == DB_SET_RANGE || fFlags == DB_GET_BOTH || fFlags == DB_GET_BOTH_RANGE)
    {
        datKey.set_data(&ssKey[0]);
        datKey.set_size(ssKey.size());
    }
    Dbt datValue;
    if (fFlags == DB_GET_BOTH || fFlags == DB_GET_BOTH_RANGE)
    {
        datValue.set_data(&ssValue[0]);
        datValue.set_size(ssValue.size());
    }
    datKey.set_flags(DB_DBT_MALLOC);
    datValue.set_flags(DB_DBT_MALLOC);
    int ret = pcursor->get(&datKey, &datValue, fFlags);
    if (ret != 0)
        return ret;
    else if (datKey.get_data() == NULL || datValue.get_data() == NULL)
        return 99999;

    // Convert to streams
    ssKey.SetType(SER_DISK);
    ssKey.clear();
    ssKey.write((char*)datKey.get_data(), datKey.get_size());
    ssValue.SetType(SER_DISK);
    ssValue.clear();
    ssValue.write((char*)datValue.get_data(), datValue.get_size());

    // Clear and free memory
    memset(datKey.get_data(), 0, datKey.get_size());
    memset(datValue.get_data(), 0, datValue.get_size());
    free(datKey.get_data());
    free(datValue.get_data());
    return 0;
}

DbTxn* CDB::GetTxn()
{
    if (!vTxn.empty())
        return vTxn.back();
    else
        return NULL;
}

bool CDB::TxnBegin()
{
    if (!pdb)
        return false;
    DbTxn* ptxn = NULL;
    int ret = dbenv.txn_begin(GetTxn(), &ptxn, DB_TXN_NOSYNC);
    if (!ptxn || ret != 0)
        return false;
    vTxn.push_back(ptxn);
    return true;
}

bool CDB::TxnCommit()
{
    if (!pdb)
        return false;
    if (vTxn.empty())
        return false;
    int ret = vTxn.back()->commit(0);
    vTxn.pop_back();
    return (ret == 0);
}

bool CDB::TxnAbort()
{
    if (!pdb)
        return false;
    if (vTxn.empty())
        return false;
    int ret = vTxn.back()->abort();
    vTxn.pop_back();
    return (ret == 0);
}

bool CDB::ReadVersion(int& nVersion)
{
    nVersion = 0;
    return Read(string("version"), nVersion);
}

bool CDB::WriteVersion(int nVersion)
{
    return Write(string("version"), nVersion);
}

CDB::CDB(const char* pszFile, const char* pszMode) : pdb(NULL)
{
    int ret;
    if (pszFile == NULL)
        return;

    fReadOnly = (!strchr(pszMode, '+') && !strchr(pszMode, 'w'));
    bool fCreate = strchr(pszMode, 'c');
    unsigned int nFlags = DB_THREAD;
    if (fCreate)
        nFlags |= DB_CREATE;

    CRITICAL_BLOCK(cs_db)
    {
        if (!fDbEnvInit)
        {
            if (fShutdown)
                return;
            string strDataDir = GetDataDir();
            string strLogDir = strDataDir + "/database";
            filesystem::create_directory(strLogDir.c_str());
            string strErrorFile = strDataDir + "/db.log";
            printf("dbenv.open strLogDir=%s strErrorFile=%s\n", strLogDir.c_str(), strErrorFile.c_str());

            dbenv.set_lg_dir(strLogDir.c_str());
            dbenv.set_lg_max(10000000);
            dbenv.set_lk_max_locks(10000);
            dbenv.set_lk_max_objects(10000);
            dbenv.set_errfile(fopen(strErrorFile.c_str(), "a")); /// debug
            dbenv.set_flags(DB_AUTO_COMMIT, 1);
            ret = dbenv.open(strDataDir.c_str(),
                             DB_CREATE     |
                             DB_INIT_LOCK  |
                             DB_INIT_LOG   |
                             DB_INIT_MPOOL |
                             DB_INIT_TXN   |
                             DB_THREAD     |
                             DB_RECOVER,
                             S_IRUSR | S_IWUSR);
            if (ret > 0)
                throw runtime_error(strprintf("CDB() : error %d opening database environment", ret));
            fDbEnvInit = true;
        }

        strFile = pszFile;
        ++mapFileUseCount[strFile];
        pdb = mapDb[strFile];
        if (pdb == NULL)
        {
            pdb = new Db(&dbenv, 0);

            ret = pdb->open(NULL,      // Txn pointer
                            pszFile,   // Filename
                            "main",    // Logical db name
                            DB_BTREE,  // Database type
                            nFlags,    // Flags
                            0);

            if (ret > 0)
            {
                delete pdb;
                pdb = NULL;
                CRITICAL_BLOCK(cs_db)
                    --mapFileUseCount[strFile];
                strFile = "";
                throw runtime_error(strprintf("CDB() : can't open database file %s, error %d", pszFile, ret));
            }

            if (fCreate && !Exists(string("version")))
            {
                bool fTmp = fReadOnly;
                fReadOnly = false;
                WriteVersion(VERSION);
                fReadOnly = fTmp;
            }

            mapDb[strFile] = pdb;
        }
    }
}

void CDB::Close()
{
    if (!pdb)
        return;
    if (!vTxn.empty())
        vTxn.front()->abort();
    vTxn.clear();
    pdb = NULL;

    // Flush database activity from memory pool to disk log
    unsigned int nMinutes = 0;
    if (fReadOnly)
        nMinutes = 1;
    if (strFile == "addr.dat")
        nMinutes = 2;
    if (strFile == "blkindex.dat" && IsInitialBlockDownload() && nBestHeight % 500 != 0)
        nMinutes = 1;
    dbenv.txn_checkpoint(0, nMinutes, 0);

    CRITICAL_BLOCK(cs_db)
        --mapFileUseCount[strFile];
}

