// Copyright (c) 2009-2010 Satoshi Nakamoto
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.

#include "ui.h"

#include "main.h"
#include "headers.h"
#include "util.h"
#include "base58.h"
#include "CMyTaskBarIcon.h"
#include "CWalletTx.h"
#include "CWalletDB.h"
#include "CSendingDialog.h"

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

DEFINE_EVENT_TYPE(wxEVT_UITHREADCALL)

CMainFrame* pframeMain = NULL;
CMyTaskBarIcon* ptaskbaricon = NULL;
bool fClosedToTray = false;
wxLocale g_locale;

//////////////////////////////////////////////////////////////////////////////
//
// Util
//

void HandleCtrlA(wxKeyEvent& event)
{
    // Ctrl-a select all
    event.Skip();
    wxTextCtrl* textCtrl = (wxTextCtrl*)event.GetEventObject();
    if (event.GetModifiers() == wxMOD_CONTROL && event.GetKeyCode() == 'A')
        textCtrl->SetSelection(-1, -1);
}

bool Is24HourTime()
{
    //char pszHourFormat[256];
    //pszHourFormat[0] = '\0';
    //GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ITIME, pszHourFormat, 256);
    //return (pszHourFormat[0] != '0');
    return true;
}

string DateStr(int64 nTime)
{
    // Can only be used safely here in the UI
    return (string)wxDateTime((time_t)nTime).FormatDate();
}

string DateTimeStr(int64 nTime)
{
    // Can only be used safely here in the UI
    wxDateTime datetime((time_t)nTime);
    if (Is24HourTime())
        return (string)datetime.Format("%x %H:%M");
    else
        return (string)datetime.Format("%x ") + itostr((datetime.GetHour() + 11) % 12 + 1) + (string)datetime.Format(":%M %p");
}

wxString GetItemText(wxListCtrl* listCtrl, int nIndex, int nColumn)
{
    // Helper to simplify access to listctrl
    wxListItem item;
    item.m_itemId = nIndex;
    item.m_col = nColumn;
    item.m_mask = wxLIST_MASK_TEXT;
    if (!listCtrl->GetItem(item))
        return "";
    return item.GetText();
}

int InsertLine(wxListCtrl* listCtrl, const wxString& str0, const wxString& str1)
{
    int nIndex = listCtrl->InsertItem(listCtrl->GetItemCount(), str0);
    listCtrl->SetItem(nIndex, 1, str1);
    return nIndex;
}

int InsertLine(wxListCtrl* listCtrl, const wxString& str0, const wxString& str1, const wxString& str2, const wxString& str3, const wxString& str4)
{
    int nIndex = listCtrl->InsertItem(listCtrl->GetItemCount(), str0);
    listCtrl->SetItem(nIndex, 1, str1);
    listCtrl->SetItem(nIndex, 2, str2);
    listCtrl->SetItem(nIndex, 3, str3);
    listCtrl->SetItem(nIndex, 4, str4);
    return nIndex;
}

int InsertLine(wxListCtrl* listCtrl, void* pdata, const wxString& str0, const wxString& str1, const wxString& str2, const wxString& str3, const wxString& str4)
{
    int nIndex = listCtrl->InsertItem(listCtrl->GetItemCount(), str0);
    listCtrl->SetItemPtrData(nIndex, (wxUIntPtr)pdata);
    listCtrl->SetItem(nIndex, 1, str1);
    listCtrl->SetItem(nIndex, 2, str2);
    listCtrl->SetItem(nIndex, 3, str3);
    listCtrl->SetItem(nIndex, 4, str4);
    return nIndex;
}

void SetItemTextColour(wxListCtrl* listCtrl, int nIndex, const wxColour& colour)
{
    // Repaint on Windows is more flickery if the colour has ever been set,
    // so don't want to set it unless it's different.  Default colour has
    // alpha 0 transparent, so our colours don't match using operator==.
    wxColour c1 = listCtrl->GetItemTextColour(nIndex);
    if (!c1.IsOk())
        c1 = wxColour(0,0,0);
    if (colour.Red() != c1.Red() || colour.Green() != c1.Green() || colour.Blue() != c1.Blue())
        listCtrl->SetItemTextColour(nIndex, colour);
}

void SetSelection(wxListCtrl* listCtrl, int nIndex)
{
    int nSize = listCtrl->GetItemCount();
    long nState = (wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
    for (int i = 0; i < nSize; i++)
        listCtrl->SetItemState(i, (i == nIndex ? nState : 0), nState);
}

int GetSelection(wxListCtrl* listCtrl)
{
    int nSize = listCtrl->GetItemCount();
    for (int i = 0; i < nSize; i++)
        if (listCtrl->GetItemState(i, wxLIST_STATE_FOCUSED))
            return i;
    return -1;
}

string HtmlEscape(const char* psz, bool fMultiLine)
{
    int len = 0;
    for (const char* p = psz; *p; p++)
    {
             if (*p == '<') len += 4;
        else if (*p == '>') len += 4;
        else if (*p == '&') len += 5;
        else if (*p == '"') len += 6;
        else if (*p == ' ' && p > psz && p[-1] == ' ' && p[1] == ' ') len += 6;
        else if (*p == '\n' && fMultiLine) len += 5;
        else
            len++;
    }
    string str;
    str.reserve(len);
    for (const char* p = psz; *p; p++)
    {
             if (*p == '<') str += "&lt;";
        else if (*p == '>') str += "&gt;";
        else if (*p == '&') str += "&amp;";
        else if (*p == '"') str += "&quot;";
        else if (*p == ' ' && p > psz && p[-1] == ' ' && p[1] == ' ') str += "&nbsp;";
        else if (*p == '\n' && fMultiLine) str += "<br>\n";
        else
            str += *p;
    }
    return str;
}

string HtmlEscape(const string& str, bool fMultiLine)
{
    return HtmlEscape(str.c_str(), fMultiLine);
}

void CalledMessageBox(const string& message, const string& caption, int style, wxWindow* parent, int x, int y, int* pnRet, bool* pfDone)
{
    *pnRet = wxMessageBox(message, caption, style, parent, x, y);
    *pfDone = true;
}

int ThreadSafeMessageBox(const string& message, const string& caption, int style, wxWindow* parent, int x, int y)
{
#ifdef __WXMSW__
    return wxMessageBox(message, caption, style, parent, x, y);
#else
    if (wxThread::IsMain() || fDaemon)
    {
        return wxMessageBox(message, caption, style, parent, x, y);
    }
    else
    {
        int nRet = 0;
        bool fDone = false;
        UIThreadCall(bind(CalledMessageBox, message, caption, style, parent, x, y, &nRet, &fDone));
        while (!fDone)
            Sleep(100);
        return nRet;
    }
#endif
}

bool ThreadSafeAskFee(int64 nFeeRequired, const string& strCaption, wxWindow* parent)
{
    if (nFeeRequired < CENT || nFeeRequired <= nTransactionFee || fDaemon)
        return true;
    string strMessage = strprintf(
        _("This transaction is over the size limit.  You can still send it for a fee of %s, "
          "which goes to the nodes that process your transaction and helps to support the network.  "
          "Do you want to pay the fee?"),
        FormatMoney(nFeeRequired).c_str());
    return (ThreadSafeMessageBox(strMessage, strCaption, wxYES_NO, parent) == wxYES);
}

void CalledSetStatusBar(const string& strText, int nField)
{
    if (nField == 0 && GetWarnings("statusbar") != "")
        return;
    if (pframeMain && pframeMain->m_statusBar)
        pframeMain->m_statusBar->SetStatusText(strText, nField);
}

void SetDefaultReceivingAddress(const string& strAddress)
{
    // Update main window address and database
    if (pframeMain == NULL)
        return;
    if (strAddress != pframeMain->m_textCtrlAddress->GetValue())
    {
        uint160 hash160;
        if (!AddressToHash160(strAddress, hash160))
            return;
        if (!mapPubKeys.count(hash160))
            return;
        CWalletDB().WriteDefaultKey(mapPubKeys[hash160]);
        pframeMain->m_textCtrlAddress->SetValue(strAddress);
    }
}


string FormatTxStatus(const CWalletTx& wtx)
{
    // Status
    if (!wtx.IsFinal())
    {
        if (wtx.nLockTime < 500000000)
            return strprintf(_("Open for %d blocks"), nBestHeight - wtx.nLockTime);
        else
            return strprintf(_("Open until %s"), DateTimeStr(wtx.nLockTime).c_str());
    }
    else
    {
        int nDepth = wtx.GetDepthInMainChain();
        if (GetAdjustedTime() - wtx.nTimeReceived > 2 * 60 && wtx.GetRequestCount() == 0)
            return strprintf(_("%d/offline?"), nDepth);
        else if (nDepth < 6)
            return strprintf(_("%d/unconfirmed"), nDepth);
        else
            return strprintf(_("%d confirmations"), nDepth);
    }
}

string SingleLine(const string& strIn)
{
    string strOut;
    bool fOneSpace = false;
    foreach(unsigned char c, strIn)
    {
        if (isspace(c))
        {
            fOneSpace = true;
        }
        else if (c > ' ')
        {
            if (fOneSpace && !strOut.empty())
                strOut += ' ';
            strOut += c;
            fOneSpace = false;
        }
    }
    return strOut;
}


unsigned int nNeedRepaint = 0;
unsigned int nLastRepaint = 0;
int64 nLastRepaintTime = 0;
int64 nRepaintInterval = 500;

void ThreadDelayedRepaint(void* parg)
{
    while (!fShutdown)
    {
        if (nLastRepaint != nNeedRepaint && GetTimeMillis() - nLastRepaintTime >= nRepaintInterval)
        {
            nLastRepaint = nNeedRepaint;
            if (pframeMain)
            {
                printf("DelayedRepaint\n");
                wxPaintEvent event;
                pframeMain->fRefresh = true;
                pframeMain->GetEventHandler()->AddPendingEvent(event);
            }
        }
        Sleep(nRepaintInterval);
    }
}

void MainFrameRepaint()
{
    // This is called by network code that shouldn't access pframeMain
    // directly because it could still be running after the UI is closed.
    if (pframeMain)
    {
        // Don't repaint too often
        static int64 nLastRepaintRequest;
        if (GetTimeMillis() - nLastRepaintRequest < 100)
        {
            nNeedRepaint++;
            return;
        }
        nLastRepaintRequest = GetTimeMillis();

        printf("MainFrameRepaint\n");
        wxPaintEvent event;
        pframeMain->fRefresh = true;
        pframeMain->GetEventHandler()->AddPendingEvent(event);
    }
}

void UIThreadCall(boost::function0<void> fn)
{
    // Call this with a function object created with bind.
    // bind needs all parameters to match the function's expected types
    // and all default parameters specified.  Some examples:
    //  UIThreadCall(bind(wxBell));
    //  UIThreadCall(bind(wxMessageBox, wxT("Message"), wxT("Title"), wxOK, (wxWindow*)NULL, -1, -1));
    //  UIThreadCall(bind(&CMainFrame::OnMenuHelpAbout, pframeMain, event));
    if (pframeMain)
    {
        wxCommandEvent event(wxEVT_UITHREADCALL);
        event.SetClientData((void*)new boost::function0<void>(fn));
        pframeMain->GetEventHandler()->AddPendingEvent(event);
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Startup folder
//

#ifdef __WXMSW__
string StartupShortcutPath()
{
    return MyGetSpecialFolderPath(CSIDL_STARTUP, true) + "\\Bitcoin.lnk";
}

bool GetStartOnSystemStartup()
{
    return filesystem::exists(StartupShortcutPath().c_str());
}

void SetStartOnSystemStartup(bool fAutoStart)
{
    // If the shortcut exists already, remove it for updating
    remove(StartupShortcutPath().c_str());

    if (fAutoStart)
    {
        CoInitialize(NULL);

        // Get a pointer to the IShellLink interface.
        IShellLink* psl = NULL;
        HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL,
                                CLSCTX_INPROC_SERVER, IID_IShellLink,
                                reinterpret_cast<void**>(&psl));

        if (SUCCEEDED(hres))
        {
            // Get the current executable path
            TCHAR pszExePath[MAX_PATH];
            GetModuleFileName(NULL, pszExePath, sizeof(pszExePath));

            // Set the path to the shortcut target
            psl->SetPath(pszExePath);
            PathRemoveFileSpec(pszExePath);
            psl->SetWorkingDirectory(pszExePath);
            psl->SetShowCmd(SW_SHOWMINNOACTIVE);

            // Query IShellLink for the IPersistFile interface for
            // saving the shortcut in persistent storage.
            IPersistFile* ppf = NULL;
            hres = psl->QueryInterface(IID_IPersistFile,
                                       reinterpret_cast<void**>(&ppf));
            if (SUCCEEDED(hres))
            {
                WCHAR pwsz[MAX_PATH];
                // Ensure that the string is ANSI.
                MultiByteToWideChar(CP_ACP, 0, StartupShortcutPath().c_str(), -1, pwsz, MAX_PATH);
                // Save the link by calling IPersistFile::Save.
                hres = ppf->Save(pwsz, TRUE);
                ppf->Release();
            }
            psl->Release();
        }
        CoUninitialize();
    }
}

#elif defined(__WXGTK__)

// Follow the Desktop Application Autostart Spec:
//  http://standards.freedesktop.org/autostart-spec/autostart-spec-latest.html

boost::filesystem::path GetAutostartDir()
{
    namespace fs = boost::filesystem;

    char* pszConfigHome = getenv("XDG_CONFIG_HOME");
    if (pszConfigHome) return fs::path(pszConfigHome) / fs::path("autostart");
    char* pszHome = getenv("HOME");
    if (pszHome) return fs::path(pszHome) / fs::path(".config/autostart");
    return fs::path();
}

boost::filesystem::path GetAutostartFilePath()
{
    return GetAutostartDir() / boost::filesystem::path("bitcoin.desktop");
}

bool GetStartOnSystemStartup()
{
    boost::filesystem::ifstream optionFile(GetAutostartFilePath());
    if (!optionFile.good())
        return false;
    // Scan through file for "Hidden=true":
    string line;
    while (!optionFile.eof())
    {
        getline(optionFile, line);
        if (line.find("Hidden") != string::npos &&
            line.find("true") != string::npos)
            return false;
    }
    optionFile.close();

    return true;
}

void SetStartOnSystemStartup(bool fAutoStart)
{
    if (!fAutoStart)
    {
        unlink(GetAutostartFilePath().native_file_string().c_str());
    }
    else
    {
        char pszExePath[MAX_PATH+1];
        memset(pszExePath, 0, sizeof(pszExePath));
        if (readlink("/proc/self/exe", pszExePath, sizeof(pszExePath)-1) == -1)
            return;

        boost::filesystem::create_directories(GetAutostartDir());

        boost::filesystem::ofstream optionFile(GetAutostartFilePath(), ios_base::out|ios_base::trunc);
        if (!optionFile.good())
        {
            wxMessageBox(_("Cannot write autostart/bitcoin.desktop file"), "Bitcoin");
            return;
        }
        // Write a bitcoin.desktop file to the autostart directory:
        optionFile << "[Desktop Entry]\n";
        optionFile << "Type=Application\n";
        optionFile << "Name=Bitcoin\n";
        optionFile << "Exec=" << pszExePath << "\n";
        optionFile << "Terminal=false\n";
        optionFile << "Hidden=false\n";
        optionFile.close();
    }
}
#else

// TODO: OSX startup stuff; see:
// http://developer.apple.com/mac/library/documentation/MacOSX/Conceptual/BPSystemStartup/Articles/CustomLogin.html

bool GetStartOnSystemStartup() { return false; }
void SetStartOnSystemStartup(bool fAutoStart) { }

#endif


void SendingDialogOnReply2(void* parg, CDataStream& vRecv)
{
    ((CSendingDialog*)parg)->OnReply2(vRecv);
}


void SendingDialogOnReply3(void* parg, CDataStream& vRecv)
{
    ((CSendingDialog*)parg)->OnReply3(vRecv);
}

void CreateMainWindow()
{
    pframeMain = new CMainFrame(NULL);
    if (GetBoolArg("-min"))
        pframeMain->Iconize(true);
#if defined(__WXGTK__) || defined(__WXMAC_OSX__)
    if (!GetBoolArg("-minimizetotray"))
        fMinimizeToTray = false;
#endif
    pframeMain->Show(true);  // have to show first to get taskbar button to hide
    if (fMinimizeToTray && pframeMain->IsIconized())
        fClosedToTray = true;
    pframeMain->Show(!fClosedToTray);
    ptaskbaricon->Show(fMinimizeToTray || fClosedToTray);
    CreateThread(ThreadDelayedRepaint, NULL);
}


