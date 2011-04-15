//////////////////////////////////////////////////////////////////////////////
//
// Start bitcoind
//

#include "init.h"
#include "util.h"

int main(int argc, char* argv[])
{
    bool fRet = false;
    fRet = AppInit(argc, argv);

    if (fRet && fDaemon)
        return 0;

    return 1;
}
