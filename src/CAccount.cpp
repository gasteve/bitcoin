#include "CAccount.h"

// Constructor
CAccount::CAccount()
{
	SetNull();
}

// SetNull
void CAccount::SetNull()
{
	vchPubKey.clear();
}
