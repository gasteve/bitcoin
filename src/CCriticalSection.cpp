// Wrapper to automatically initialize critical sections

#include "CCriticalSection.h"

#ifdef __WXMSW__

CCriticalSection::CCriticalSection() { InitializeCriticalSection(&cs); }
CCriticalSection::~CCriticalSection() { DeleteCriticalSection(&cs); }
void CCriticalSection::Enter() { EnterCriticalSection(&cs); }
void CCriticalSection::Leave() { LeaveCriticalSection(&cs); }
bool CCriticalSection::TryEnter() { return TryEnterCriticalSection(&cs); }

#else

CCriticalSection::CCriticalSection() { }
CCriticalSection::~CCriticalSection() { }
void CCriticalSection::Enter() { mutex.lock(); }
void CCriticalSection::Leave() { mutex.unlock(); }
bool CCriticalSection::TryEnter() { return mutex.try_lock(); }

#endif
