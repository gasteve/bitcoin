//////////////////////////////////////////////////////////////////////////////
//
// Start bitcoind
//

#include "init.h"
#include "util.h"

int main(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++)
        if (!IsSwitchChar(argv[i][0]))
            fCommandLine = true;
    fDaemon = !fCommandLine;

#ifdef __WXGTK__
    if (!fCommandLine)
    {
        // Daemonize
        pid_t pid = fork();
        if (pid < 0)
        {
            fprintf(stderr, "Error: fork() returned %d errno %d\n", pid, errno);
            return 1;
        }
        if (pid > 0)
            pthread_exit((void*)0);
    }
#endif

    if (!AppInit(argc, argv))
        return 1;

    while (!fShutdown)
        Sleep(1000000);
    return 0;
}

