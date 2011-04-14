#include "CAccountingEntry.h"

CAccountingEntry::CAccountingEntry()
{
	SetNull();
}

void CAccountingEntry::SetNull()
{
	nCreditDebit = 0;
	nTime = 0;
	strAccount.clear();
	strOtherAccount.clear();
	strComment.clear();
}
