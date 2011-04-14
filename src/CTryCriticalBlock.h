#ifndef CTRYCRITICALBLOCK_H
#define CTRYCRITICALBLOCK_H

class CCriticalSection;

class CTryCriticalBlock
{
protected:
    CCriticalSection* pcs;
public:
    CTryCriticalBlock(CCriticalSection& csIn);
    ~CTryCriticalBlock();
    bool Entered();
};

#define TRY_CRITICAL_BLOCK(cs)     \
    for (bool fcriticalblockonce=true; fcriticalblockonce; assert(("break caught by TRY_CRITICAL_BLOCK!", !fcriticalblockonce)), fcriticalblockonce=false)  \
    for (CTryCriticalBlock criticalblock(cs); fcriticalblockonce && (fcriticalblockonce = criticalblock.Entered()) && (cs.pszFile=__FILE__, cs.nLine=__LINE__, true); fcriticalblockonce=false, cs.pszFile=NULL, cs.nLine=0)

#endif
