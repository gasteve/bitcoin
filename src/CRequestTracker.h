#ifndef CREQUESTTRACKER_H
#define CREQUESTTRACKER_H

#include "headers.h"

class CDataStream;

class CRequestTracker
{
public:
    void (*fn)(void*, CDataStream&);
    void* param1;

    explicit CRequestTracker(void (*fnIn)(void*, CDataStream&)=NULL, void* param1In=NULL);
    bool IsNull();
};

#endif
