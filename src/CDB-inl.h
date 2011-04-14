#include "serialize.h"
#include "CDataStream.h"

template<typename K, typename T>
bool CDB::Read(const K& key, T& value)
{
    if (!pdb)
        return false;

    // Key
    CDataStream ssKey(SER_DISK);
    ssKey.reserve(1000);
    ssKey << key;
    Dbt datKey(&ssKey[0], ssKey.size());

    // Read
    Dbt datValue;
    datValue.set_flags(DB_DBT_MALLOC);
    int ret = pdb->get(GetTxn(), &datKey, &datValue, 0);
    memset(datKey.get_data(), 0, datKey.get_size());
    if (datValue.get_data() == NULL)
        return false;

    // Unserialize value
    CDataStream ssValue((char*)datValue.get_data(), (char*)datValue.get_data() + datValue.get_size(), SER_DISK);
    ssValue >> value;

    // Clear and free memory
    memset(datValue.get_data(), 0, datValue.get_size());
    free(datValue.get_data());
    return (ret == 0);
}

template<typename K, typename T>
bool CDB::Write(const K& key, const T& value, bool fOverwrite)
{
    if (!pdb)
        return false;
    if (fReadOnly)
        assert(("Write called on database in read-only mode", false));

    // Key
    CDataStream ssKey(SER_DISK);
    ssKey.reserve(1000);
    ssKey << key;
    Dbt datKey(&ssKey[0], ssKey.size());

    // Value
    CDataStream ssValue(SER_DISK);
    ssValue.reserve(10000);
    ssValue << value;
    Dbt datValue(&ssValue[0], ssValue.size());

    // Write
    int ret = pdb->put(GetTxn(), &datKey, &datValue, (fOverwrite ? 0 : DB_NOOVERWRITE));

    // Clear memory in case it was a private key
    memset(datKey.get_data(), 0, datKey.get_size());
    memset(datValue.get_data(), 0, datValue.get_size());
    return (ret == 0);
}

template<typename K>
bool CDB::Erase(const K& key)
{
    if (!pdb)
        return false;
    if (fReadOnly)
        assert(("Erase called on database in read-only mode", false));

    // Key
    CDataStream ssKey(SER_DISK);
    ssKey.reserve(1000);
    ssKey << key;
    Dbt datKey(&ssKey[0], ssKey.size());

    // Erase
    int ret = pdb->del(GetTxn(), &datKey, 0);

    // Clear memory
    memset(datKey.get_data(), 0, datKey.get_size());
    return (ret == 0 || ret == DB_NOTFOUND);
}

template<typename K>
bool CDB::Exists(const K& key)
{
    if (!pdb)
        return false;

    // Key
    CDataStream ssKey(SER_DISK);
    ssKey.reserve(1000);
    ssKey << key;
    Dbt datKey(&ssKey[0], ssKey.size());

    // Exists
    int ret = pdb->exists(GetTxn(), &datKey, 0);

    // Clear memory
    memset(datKey.get_data(), 0, datKey.get_size());
    return (ret == 0);
}
