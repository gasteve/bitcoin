//
// Automatic closing wrapper for FILE*
//  - Will automatically close the file when it goes out of scope if not null.
//  - If you're returning the file pointer, return file.release().
//  - If you need to close the file early, use file.fclose() instead of fclose(file).
//

template<typename T>
unsigned int CAutoFile::GetSerializeSize(const T& obj)
{
    // Tells the size of the object if serialized to this stream
    return ::GetSerializeSize(obj, nType, nVersion);
}

template<typename T>
CAutoFile& CAutoFile::operator<<(const T& obj)
{
    // Serialize to this stream
    if (!file)
        throw std::ios_base::failure("CAutoFile::operator<< : file handle is NULL");
    ::Serialize(*this, obj, nType, nVersion);
    return (*this);
}

template<typename T>
CAutoFile& CAutoFile::operator>>(T& obj)
{
    // Unserialize from this stream
    if (!file)
        throw std::ios_base::failure("CAutoFile::operator>> : file handle is NULL");
    ::Unserialize(*this, obj, nType, nVersion);
    return (*this);
}

