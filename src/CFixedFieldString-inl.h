template<std::size_t LEN>
CFixedFieldString<LEN>::CFixedFieldString(const string& str) : pcstr(&str), pstr(NULL) { }

template<std::size_t LEN>
CFixedFieldString<LEN>::CFixedFieldString(string& str) : pcstr(&str), pstr(&str) { }

template<std::size_t LEN>
unsigned int CFixedFieldString<LEN>::GetSerializeSize(int, int) const
{
    return LEN;
}

template<std::size_t LEN>
template<typename Stream>
void CFixedFieldString<LEN>::Serialize(Stream& s, int, int) const
{
    char pszBuf[LEN];
    strncpy(pszBuf, pcstr->c_str(), LEN);
    s.write(pszBuf, LEN);
}

template<std::size_t LEN>
template<typename Stream>
void CFixedFieldString<LEN>::Unserialize(Stream& s, int, int)
{
    if (pstr == NULL)
        throw std::ios_base::failure("CFixedFieldString::Unserialize : trying to unserialize to const string");
    char pszBuf[LEN+1];
    s.read(pszBuf, LEN);
    pszBuf[LEN] = '\0';
    *pstr = pszBuf;
}
