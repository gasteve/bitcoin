#include "CCriticalBlock.h"

CCriticalBlock::CCriticalBlock(CCriticalSection& csIn) { pcs = &csIn; pcs->Enter(); }
CCriticalBlock::~CCriticalBlock() { pcs->Leave(); }
