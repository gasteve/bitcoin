template<typename Stream>
void CFlatData::Serialize(Stream& s, int, int) const
{
    s.write(pbegin, pend - pbegin);
}

template<typename Stream>
void CFlatData::Unserialize(Stream& s, int, int)
{
    s.read(pbegin, pend - pbegin);
}
