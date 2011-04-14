#ifndef CMESSAGEHEADER_H
#define CMESSAGEHEADER_H

#include "headers.h"
#include "serialize.h"
#include "util.h"

//
// Message header
//  (4) message start
//  (12) command
//  (4) size
//  (4) checksum

extern char pchMessageStart[4];

class CMessageHeader
{
public:
    enum { COMMAND_SIZE=12 };
    char pchMessageStart[sizeof(::pchMessageStart)];
    char pchCommand[COMMAND_SIZE];
    unsigned int nMessageSize;
    unsigned int nChecksum;

    CMessageHeader();
    CMessageHeader(const char* pszCommand, unsigned int nMessageSizeIn);
    string GetCommand();
    bool IsValid();

    IMPLEMENT_SERIALIZE
    (
        READWRITE(FLATDATA(pchMessageStart));
        READWRITE(FLATDATA(pchCommand));
        READWRITE(nMessageSize);
        if (nVersion >= 209)
            READWRITE(nChecksum);
    )
};

#endif
