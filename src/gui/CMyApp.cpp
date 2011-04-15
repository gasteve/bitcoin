//////////////////////////////////////////////////////////////////////////////
//
// CMyApp
//

#include "CMyApp.h"

#include "ui.h"
#include "util.h"
#include "init.h"

IMPLEMENT_APP(CMyApp)

bool CMyApp::Initialize(int& argc, wxChar** argv)
{
    for (int i = 1; i < argc; i++)
        if (!IsSwitchChar(argv[i][0]))
            fCommandLine = true;

    if (!fCommandLine)
    {
        // wxApp::Initialize will remove environment-specific parameters,
        // so it's too early to call ParseParameters yet
        for (int i = 1; i < argc; i++)
        {
            wxString str = argv[i];
            #ifdef __WXMSW__
            if (str.size() >= 1 && str[0] == '/')
                str[0] = '-';
            char pszLower[MAX_PATH];
            strlcpy(pszLower, str.c_str(), sizeof(pszLower));
            strlwr(pszLower);
            str = pszLower;
            #endif
            if (str == "-daemon")
                fDaemon = true;
        }
    }

#ifdef __WXGTK__
    if (fDaemon || fCommandLine)
    {
        // Call the original Initialize while suppressing error messages
        // and ignoring failure.  If unable to initialize GTK, it fails
        // near the end so hopefully the last few things don't matter.
        {
            wxLogNull logNo;
            wxApp::Initialize(argc, argv);
        }

        if (fDaemon)
        {
            // Daemonize
            pid_t pid = fork();
            if (pid < 0)
            {
                fprintf(stderr, "Error: fork() returned %d errno %d\n", pid, errno);
                return false;
            }
            if (pid > 0)
                pthread_exit((void*)0);

            pid_t sid = setsid();
            if (sid < 0)
                fprintf(stderr, "Error: setsid() returned %d errno %d\n", sid, errno);
        }

        return true;
    }
#endif

    return wxApp::Initialize(argc, argv);
}

bool CMyApp::OnInit()
{
#if defined(__WXMSW__) && defined(__WXDEBUG__) && defined(GUI)
    // Disable malfunctioning wxWidgets debug assertion
    extern int g_isPainting;
    g_isPainting = 10000;
#endif
#ifdef GUI
    wxImage::AddHandler(new wxPNGHandler);
#endif
#if defined(__WXMSW__ ) || defined(__WXMAC_OSX__)
    SetAppName("Bitcoin");
#else
    SetAppName("bitcoin");
#endif
#ifdef __WXMSW__
#if wxUSE_UNICODE
    // Hack to set wxConvLibc codepage to UTF-8 on Windows,
    // may break if wxMBConv_win32 implementation in strconv.cpp changes.
    class wxMBConv_win32 : public wxMBConv
    {
    public:
        long m_CodePage;
        size_t m_minMBCharWidth;
    };
    if (((wxMBConv_win32*)&wxConvLibc)->m_CodePage == CP_ACP)
        ((wxMBConv_win32*)&wxConvLibc)->m_CodePage = CP_UTF8;
#endif
#endif

    // Load locale/<lang>/LC_MESSAGES/bitcoin.mo language file
    g_locale.Init(wxLANGUAGE_DEFAULT, 0);
    g_locale.AddCatalogLookupPathPrefix("locale");
#ifndef __WXMSW__
    g_locale.AddCatalogLookupPathPrefix("/usr/share/locale");
    g_locale.AddCatalogLookupPathPrefix("/usr/local/share/locale");
#endif
    g_locale.AddCatalog("wxstd"); // wxWidgets standard translations, if any
    g_locale.AddCatalog("bitcoin");

    return AppInit(argc, argv);
}

int CMyApp::OnExit()
{
    Shutdown(NULL);
    return wxApp::OnExit();
}

bool CMyApp::OnExceptionInMainLoop()
{
    try
    {
        throw;
    }
    catch (std::exception& e)
    {
        PrintException(&e, "CMyApp::OnExceptionInMainLoop()");
        wxLogWarning("Exception %s %s", typeid(e).name(), e.what());
        Sleep(1000);
        throw;
    }
    catch (...)
    {
        PrintException(NULL, "CMyApp::OnExceptionInMainLoop()");
        wxLogWarning("Unknown exception");
        Sleep(1000);
        throw;
    }
    return true;
}

void CMyApp::OnUnhandledException()
{
    // this shows how we may let some exception propagate uncaught
    try
    {
        throw;
    }
    catch (std::exception& e)
    {
        PrintException(&e, "CMyApp::OnUnhandledException()");
        wxLogWarning("Exception %s %s", typeid(e).name(), e.what());
        Sleep(1000);
        throw;
    }
    catch (...)
    {
        PrintException(NULL, "CMyApp::OnUnhandledException()");
        wxLogWarning("Unknown exception");
        Sleep(1000);
        throw;
    }
}

void CMyApp::OnFatalException()
{
    wxMessageBox(_("Program has crashed and will terminate.  "), "Bitcoin", wxOK | wxICON_ERROR);
}
