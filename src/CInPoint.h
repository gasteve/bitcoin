#ifndef CINPOINT_H
#define CINPOINT_H

#include "CTransaction.h"

class CInPoint
{
public:
    CTransaction* ptx;
    unsigned int n;

    CInPoint();
    CInPoint(CTransaction* ptxIn, unsigned int nIn);
    void SetNull();
    bool IsNull() const;
};

#endif
