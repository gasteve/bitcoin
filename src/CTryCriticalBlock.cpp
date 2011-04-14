#include "CTryCriticalBlock.h"
#include "headers.h"
#include "CCriticalSection.h"

CTryCriticalBlock::CTryCriticalBlock(CCriticalSection& csIn) { pcs = (csIn.TryEnter() ? &csIn : NULL); }
CTryCriticalBlock::~CTryCriticalBlock() { if (pcs) pcs->Leave(); }
bool CTryCriticalBlock::Entered() { return pcs != NULL; }
