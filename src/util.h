// Copyright (c) 2009-2010 Satoshi Nakamoto
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.

#ifndef UTIL_H
#define UTIL_H

#include "headers.h"
#include "serialize.h"
#include "uint256.h"

#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef __int64  int64;
typedef unsigned __int64  uint64;
#else
typedef long long  int64;
typedef unsigned long long  uint64;
#endif
#if defined(_MSC_VER) && _MSC_VER < 1300
#define for  if (false) ; else for
#endif
#ifndef _MSC_VER
#define __forceinline  inline
#endif

#define foreach             BOOST_FOREACH
#define loop                for (;;)
#define BEGIN(a)            ((char*)&(a))
#define END(a)              ((char*)&((&(a))[1]))
#define UBEGIN(a)           ((unsigned char*)&(a))
#define UEND(a)             ((unsigned char*)&((&(a))[1]))
#define ARRAYLEN(array)     (sizeof(array)/sizeof((array)[0]))
#define printf              OutputDebugStringF

#ifdef snprintf
#undef snprintf
#endif
#define snprintf my_snprintf

#ifndef PRI64d
#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__MSVCRT__)
#define PRI64d  "I64d"
#define PRI64u  "I64u"
#define PRI64x  "I64x"
#else
#define PRI64d  "lld"
#define PRI64u  "llu"
#define PRI64x  "llx"
#endif
#endif

// This is needed because the foreach macro can't get over the comma in pair<t1, t2>
#define PAIRTYPE(t1, t2)    pair<t1, t2>

// Used to bypass the rule against non-const reference to temporary
// where it makes sense with wrappers such as CFlatData or CTxDB
template<typename T>
inline T& REF(const T& val);

// Align by increasing pointer, must have extra space at end of buffer
template <size_t nBytes, typename T>
T* alignup(T* p);

#ifdef __WXMSW__
#define MSG_NOSIGNAL        0
#define MSG_DONTWAIT        0
#ifndef UINT64_MAX
#define UINT64_MAX          _UI64_MAX
#define INT64_MAX           _I64_MAX
#define INT64_MIN           _I64_MIN
#endif
#ifndef S_IRUSR
#define S_IRUSR             0400
#define S_IWUSR             0200
#endif
#define unlink              _unlink
typedef int socklen_t;
#else
#define WSAGetLastError()   errno
#define WSAEWOULDBLOCK      EWOULDBLOCK
#define WSAEMSGSIZE         EMSGSIZE
#define WSAEINTR            EINTR
#define WSAEINPROGRESS      EINPROGRESS
#define WSAEADDRINUSE       EADDRINUSE
#define WSAENOTSOCK         EBADF
#define INVALID_SOCKET      (SOCKET)(~0)
#define SOCKET_ERROR        -1
typedef u_int SOCKET;
#define _vsnprintf(a,b,c,d) vsnprintf(a,b,c,d)
#define strlwr(psz)         to_lower(psz)
#define _strlwr(psz)        to_lower(psz)
#define MAX_PATH            1024
#define Beep(n1,n2)         (0)
inline void Sleep(int64 n);
#endif

extern map<string, string> mapArgs;
extern map<string, vector<string> > mapMultiArgs;
extern bool fDebug;
extern bool fPrintToConsole;
extern bool fPrintToDebugger;
extern char pszSetDataDir[MAX_PATH];
extern bool fRequestShutdown;
extern bool fShutdown;
extern bool fDaemon;
extern bool fCommandLine;
extern string strMiscWarning;
extern bool fTestNet;
extern bool fNoListen;

inline int myclosesocket(SOCKET& hSocket);

#define closesocket(s)      myclosesocket(s)

#ifndef GUI
inline const char* _(const char* psz);
#endif

void RandAddSeed();
void RandAddSeedPerfmon();
int OutputDebugStringF(const char* pszFormat, ...);
int my_snprintf(char* buffer, size_t limit, const char* format, ...);
string strprintf(const char* format, ...);
bool error(const char* format, ...);
void LogException(std::exception* pex, const char* pszThread);
void PrintException(std::exception* pex, const char* pszThread);
void PrintExceptionContinue(std::exception* pex, const char* pszThread);
void ParseString(const string& str, char c, vector<string>& v);
string FormatMoney(int64 n, bool fPlus=false);
bool ParseMoney(const string& str, int64& nRet);
bool ParseMoney(const char* pszIn, int64& nRet);
vector<unsigned char> ParseHex(const char* psz);
vector<unsigned char> ParseHex(const string& str);
void ParseParameters(int argc, char* argv[]);
const char* wxGetTranslation(const char* psz);
bool WildcardMatch(const char* psz, const char* mask);
bool WildcardMatch(const string& str, const string& mask);
int GetFilesize(FILE* file);
void GetDataDir(char* pszDirRet);
string GetConfigFile();
void ReadConfigFile(map<string, string>& mapSettingsRet, map<string, vector<string> >& mapMultiSettingsRet);
#ifdef __WXMSW__
string MyGetSpecialFolderPath(int nFolder, bool fCreate);
#endif
string GetDefaultDataDir();
string GetDataDir();
void ShrinkDebugFile();
int GetRandInt(int nMax);
uint64 GetRand(uint64 nMax);
int64 GetTime();
int64 GetAdjustedTime();
void AddTimeData(unsigned int ip, int64 nTime);

inline string i64tostr(int64 n);
inline string itostr(int n);
inline int64 atoi64(const char* psz);
inline int64 atoi64(const string& str);
inline int atoi(const string& str);
inline int roundint(double d);
inline int64 roundint64(double d);
inline int64 abs64(int64 n);
template<typename T> string HexStr(const T itbegin, const T itend, bool fSpaces=false);
inline string HexStr(const vector<unsigned char>& vch, bool fSpaces=false);
template<typename T> string HexNumStr(const T itbegin, const T itend, bool f0x=true);

inline string HexNumStr(const vector<unsigned char>& vch, bool f0x=true);

template<typename T> void PrintHex(const T pbegin, const T pend, const char* pszFormat="%s", bool fSpaces=true);
inline void PrintHex(const vector<unsigned char>& vch, const char* pszFormat="%s", bool fSpaces=true);
inline int64 GetPerformanceCounter();
inline int64 GetTimeMillis();
inline string DateTimeStrFormat(const char* pszFormat, int64 nTime);
template<typename T> void skipspaces(T& it);
inline bool IsSwitchChar(char c);
inline string GetArg(const string& strArg, const string& strDefault);
inline int64 GetArg(const string& strArg, int64 nDefault);
inline bool GetBoolArg(const string& strArg);
inline string FormatVersion(int nVersion);
inline void heapchk();

// Randomize the stack to help protect against buffer overrun exploits
#define IMPLEMENT_RANDOMIZE_STACK(ThreadFn)     \
    {                                           \
        static char nLoops;                     \
        if (nLoops <= 0)                        \
            nLoops = GetRand(20) + 1;           \
        if (nLoops-- > 1)                       \
        {                                       \
            ThreadFn;                           \
            return;                             \
        }                                       \
    }

#define CATCH_PRINT_EXCEPTION(pszFn)     \
    catch (std::exception& e) {          \
        PrintException(&e, (pszFn));     \
    } catch (...) {                      \
        PrintException(NULL, (pszFn));   \
    }

template<typename T1> inline uint256 Hash(const T1 pbegin, const T1 pend);

template<typename T1, typename T2>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end);

template<typename T1, typename T2, typename T3>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end,
                    const T3 p3begin, const T3 p3end);

template<typename T>
uint256 SerializeHash(const T& obj, int nType=SER_GETHASH, int nVersion=VERSION);

inline uint160 Hash160(const vector<unsigned char>& vch);

// Note: It turns out we might have been able to use boost::thread
// by using TerminateThread(boost::thread.native_handle(), 0);
#ifdef __WXMSW__

typedef HANDLE pthread_t;
inline pthread_t CreateThread(void(*pfn)(void*), void* parg, bool fWantHandle=false);
inline void SetThreadPriority(int nPriority);

#else

inline pthread_t CreateThread(void(*pfn)(void*), void* parg, bool fWantHandle=false);

#define THREAD_PRIORITY_LOWEST          PRIO_MAX
#define THREAD_PRIORITY_BELOW_NORMAL    2
#define THREAD_PRIORITY_NORMAL          0
#define THREAD_PRIORITY_ABOVE_NORMAL    0

inline void SetThreadPriority(int nPriority);
inline bool TerminateThread(pthread_t hthread, unsigned int nExitCode);
inline void ExitThread(unsigned int nExitCode);

#endif

inline bool AffinityBugWorkaround(void(*pfn)(void*));

#include "util-inl.h"

#ifdef GUI
#include "gui/uibase.h"
#include "gui/ui.h"
#else
#include "cli/noui.h"
#endif

#endif
