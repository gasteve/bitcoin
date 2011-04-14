//
// Automatic closing wrapper for FILE*
//  - Will automatically close the file when it goes out of scope if not null.
//  - If you're returning the file pointer, return file.release().
//  - If you need to close the file early, use file.fclose() instead of fclose(file).
//

#include "CAutoFile.h"

CAutoFile::CAutoFile(FILE* filenew, int nTypeIn, int nVersionIn)
{
    file = filenew;
    nType = nTypeIn;
    nVersion = nVersionIn;
    state = 0;
    exceptmask = ios::badbit | ios::failbit;
}

CAutoFile::~CAutoFile()
{
    fclose();
}

void CAutoFile::fclose()
{
    if (file != NULL && file != stdin && file != stdout && file != stderr)
        ::fclose(file);
    file = NULL;
}

FILE* CAutoFile::release()             { FILE* ret = file; file = NULL; return ret; }
CAutoFile::operator FILE*()            { return file; }
FILE* CAutoFile::operator->()          { return file; }
FILE& CAutoFile::operator*()           { return *file; }
FILE** CAutoFile::operator&()          { return &file; }
FILE* CAutoFile::operator=(FILE* pnew) { return file = pnew; }
bool CAutoFile::operator!()            { return (file == NULL); }


//
// Stream subset
//
void CAutoFile::setstate(short bits, const char* psz)
{
    state |= bits;
    if (state & exceptmask)
        throw std::ios_base::failure(psz);
}

bool CAutoFile::fail() const            { return state & (ios::badbit | ios::failbit); }
bool CAutoFile::good() const            { return state == 0; }
void CAutoFile::clear(short n)      { state = n; }
short CAutoFile::exceptions()           { return exceptmask; }
short CAutoFile::exceptions(short mask) { short prev = exceptmask; exceptmask = mask; setstate(0, "CAutoFile"); return prev; }

void CAutoFile::SetType(int n)          { nType = n; }
int CAutoFile::GetType()                { return nType; }
void CAutoFile::SetVersion(int n)       { nVersion = n; }
int CAutoFile::GetVersion()             { return nVersion; }
void CAutoFile::ReadVersion()           { *this >> nVersion; }
void CAutoFile::WriteVersion()          { *this << nVersion; }

CAutoFile& CAutoFile::read(char* pch, int nSize)
{
    if (!file)
        throw std::ios_base::failure("CAutoFile::read : file handle is NULL");
    if (fread(pch, 1, nSize, file) != nSize)
        setstate(ios::failbit, feof(file) ? "CAutoFile::read : end of file" : "CAutoFile::read : fread failed");
    return (*this);
}

CAutoFile& CAutoFile::write(const char* pch, int nSize)
{
    if (!file)
        throw std::ios_base::failure("CAutoFile::write : file handle is NULL");
    if (fwrite(pch, 1, nSize, file) != nSize)
        setstate(ios::failbit, "CAutoFile::write : write failed");
    return (*this);
}
