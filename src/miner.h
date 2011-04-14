#ifndef MINER_H
#define MINER_H

#include "uint256.h"
class CBlock;
class CBlockIndex;
class CReserveKey;

void GenerateBitcoins(bool fGenerate);
void ThreadBitcoinMiner(void* parg);
CBlock* CreateNewBlock(CReserveKey& reservekey);
void IncrementExtraNonce(CBlock* pblock, CBlockIndex* pindexPrev, unsigned int& nExtraNonce, int64& nPrevTime);
void FormatHashBuffers(CBlock* pblock, char* pmidstate, char* pdata, char* phash1);
bool CheckWork(CBlock* pblock, CReserveKey& reservekey);
void BitcoinMiner();
bool CheckProofOfWork(uint256 hash, unsigned int nBits);

#endif
