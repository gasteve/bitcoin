#include "CRequestTracker.h"

CRequestTracker::CRequestTracker(void (*fnIn)(void*, CDataStream&), void* param1In)
{
	fn = fnIn;
	param1 = param1In;
}

bool CRequestTracker::IsNull()
{
	return fn == NULL;
}
