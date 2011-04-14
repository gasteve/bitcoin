#ifndef CCRITICALSECTION_H
#define CCRITICALSECTION_H

#include "headers.h"

// Wrapper to automatically initialize critical sections
class CCriticalSection
{
#ifdef __WXMSW__
protected:
    CRITICAL_SECTION cs;
public:
    explicit CCriticalSection();
    ~CCriticalSection();
    void Enter();
    void Leave();
    bool TryEnter();
#else
protected:
    boost::interprocess::interprocess_recursive_mutex mutex;
public:
    explicit CCriticalSection();
    ~CCriticalSection();
    void Enter();
    void Leave();
    bool TryEnter();
#endif
public:
    const char* pszFile;
    int nLine;
};

#endif
