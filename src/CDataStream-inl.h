//
// Double ended buffer combining vector and stream-like interfaces.
// >> and << read and write unformatted data using the above serialization templates.
// Fills with data in linear time; some stringstream implementations take N^2 time.
//

inline void CDataStream::Compact()
{
    vch.erase(vch.begin(), vch.begin() + nReadPos);
    nReadPos = 0;
}

template<typename Stream>
void CDataStream::Serialize(Stream& s, int nType, int nVersion) const
{
    // Special case: stream << stream concatenates like stream += stream
    if (!vch.empty())
        s.write((char*)&vch[0], vch.size() * sizeof(vch[0]));
}

template<typename T>
unsigned int CDataStream::GetSerializeSize(const T& obj)
{
    // Tells the size of the object if serialized to this stream
    return ::GetSerializeSize(obj, nType, nVersion);
}

template<typename T>
CDataStream& CDataStream::operator<<(const T& obj)
{
    // Serialize to this stream
    ::Serialize(*this, obj, nType, nVersion);
    return (*this);
}

template<typename T>
CDataStream& CDataStream::operator>>(T& obj)
{
    // Unserialize from this stream
    ::Unserialize(*this, obj, nType, nVersion);
    return (*this);
}

