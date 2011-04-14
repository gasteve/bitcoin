template<typename T>
bool CWalletDB::ReadSetting(const string& strKey, T& value)
{
    return Read(make_pair(string("setting"), strKey), value);
}

template<typename T>
bool CWalletDB::WriteSetting(const string& strKey, const T& value)
{
    nWalletDBUpdated++;
    return Write(make_pair(string("setting"), strKey), value);
}
