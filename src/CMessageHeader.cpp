#include "CMessageHeader.h"

//
// Message header
//  (4) message start
//  (12) command
//  (4) size
//  (4) checksum

extern char pchMessageStart[4];

CMessageHeader::CMessageHeader()
{
	memcpy(pchMessageStart, ::pchMessageStart, sizeof(pchMessageStart));
	memset(pchCommand, 0, sizeof(pchCommand));
	pchCommand[1] = 1;
	nMessageSize = -1;
	nChecksum = 0;
}

CMessageHeader::CMessageHeader(const char* pszCommand, unsigned int nMessageSizeIn)
{
	memcpy(pchMessageStart, ::pchMessageStart, sizeof(pchMessageStart));
	strncpy(pchCommand, pszCommand, COMMAND_SIZE);
	nMessageSize = nMessageSizeIn;
	nChecksum = 0;
}

string CMessageHeader::GetCommand()
{
	if (pchCommand[COMMAND_SIZE-1] == 0)
		return string(pchCommand, pchCommand + strlen(pchCommand));
	else
		return string(pchCommand, pchCommand + COMMAND_SIZE);
}

bool CMessageHeader::IsValid()
{
	// Check start string
	if (memcmp(pchMessageStart, ::pchMessageStart, sizeof(pchMessageStart)) != 0)
		return false;

	// Check the command string for errors
	for (char* p1 = pchCommand; p1 < pchCommand + COMMAND_SIZE; p1++)
	{
		if (*p1 == 0)
		{
			// Must be all zeros after the first zero
			for (; p1 < pchCommand + COMMAND_SIZE; p1++)
				if (*p1 != 0)
					return false;
		}
		else if (*p1 < ' ' || *p1 > 0x7E)
			return false;
	}

	// Message size
	if (nMessageSize > MAX_SIZE)
	{
		printf("CMessageHeader::IsValid() : (%s, %u bytes) nMessageSize > MAX_SIZE\n", GetCommand().c_str(), nMessageSize);
		return false;
	}

	return true;
}

