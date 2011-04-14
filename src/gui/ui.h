// Copyright (c) 2009-2010 Satoshi Nakamoto
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.

#ifndef UI_H
#define UI_H

#include "headers.h"
#include "util.h"
#include "uibase.h"
#include "CMainFrame.h"

#include "xpm/addressbook16.xpm"
#include "xpm/addressbook20.xpm"
#include "xpm/bitcoin16.xpm"
#include "xpm/bitcoin20.xpm"
#include "xpm/bitcoin32.xpm"
#include "xpm/bitcoin48.xpm"
#include "xpm/bitcoin80.xpm"
#include "xpm/check.xpm"
#include "xpm/send16.xpm"
#include "xpm/send16noshadow.xpm"
#include "xpm/send20.xpm"
#include "xpm/about.xpm"

class CDataStream;
class CMyTaskBarIcon;

DECLARE_EVENT_TYPE(wxEVT_UITHREADCALL, -1)

extern wxLocale g_locale;
extern CMainFrame* pframeMain;
extern CMyTaskBarIcon* ptaskbaricon;
extern bool fClosedToTray;
extern int64 nLastRepaintTime;
extern unsigned int nLastRepaint;
extern int64 nRepaintInterval;
extern unsigned int nNeedRepaint;

void HandleCtrlA(wxKeyEvent& event);
void UIThreadCall(boost::function0<void>);
int ThreadSafeMessageBox(const string& message, const string& caption="Message", int style=wxOK, wxWindow* parent=NULL, int x=-1, int y=-1);
bool ThreadSafeAskFee(int64 nFeeRequired, const string& strCaption, wxWindow* parent);
void CalledSetStatusBar(const string& strText, int nField);
void MainFrameRepaint();
void CreateMainWindow();
void SetStartOnSystemStartup(bool fAutoStart);
inline int MyMessageBox(const wxString& message, const wxString& caption="Message", int style=wxOK, wxWindow* parent=NULL, int x=-1, int y=-1);

void SendingDialogStartTransfer(void* parg);
void SendingDialogOnReply2(void* parg, CDataStream& vRecv);
void SendingDialogOnReply3(void* parg, CDataStream& vRecv);
int InsertLine(wxListCtrl* listCtrl, const wxString& str0, const wxString& str1);
int InsertLine(wxListCtrl* listCtrl, const wxString& str0, const wxString& str1, const wxString& str2, const wxString& str3, const wxString& str4);
int InsertLine(wxListCtrl* listCtrl, void* pdata, const wxString& str0, const wxString& str1, const wxString& str2, const wxString& str3, const wxString& str4);

void HandleCtrlA(wxKeyEvent& event);
bool Is24HourTime();
string DateStr(int64 nTime);
string DateTimeStr(int64 nTime);
wxString GetItemText(wxListCtrl* listCtrl, int nIndex, int nColumn);
int InsertLine(wxListCtrl* listCtrl, const wxString& str0, const wxString& str1);
int InsertLine(wxListCtrl* listCtrl, const wxString& str0, const wxString& str1, const wxString& str2, const wxString& str3, const wxString& str4);
int InsertLine(wxListCtrl* listCtrl, void* pdata, const wxString& str0, const wxString& str1, const wxString& str2, const wxString& str3, const wxString& str4);
void SetItemTextColour(wxListCtrl* listCtrl, int nIndex, const wxColour& colour);
void SetSelection(wxListCtrl* listCtrl, int nIndex);
int GetSelection(wxListCtrl* listCtrl);
string HtmlEscape(const char* psz, bool fMultiLine=false);
string HtmlEscape(const string& str, bool fMultiLine=false);
void CalledMessageBox(const string& message, const string& caption, int style, wxWindow* parent, int x, int y, int* pnRet, bool* pfDone);
int ThreadSafeMessageBox(const string& message, const string& caption, int style, wxWindow* parent, int x, int y);
bool ThreadSafeAskFee(int64 nFeeRequired, const string& strCaption, wxWindow* parent);
void CalledSetStatusBar(const string& strText, int nField);
void SetDefaultReceivingAddress(const string& strAddress);
string FormatTxStatus(const CWalletTx& wtx);
string SingleLine(const string& strIn);
void ThreadDelayedRepaint(void* parg);
void MainFrameRepaint();
void UIThreadCall(boost::function0<void> fn);

#ifdef __WXMSW__
string StartupShortcutPath();
bool GetStartOnSystemStartup();
void SetStartOnSystemStartup(bool fAutoStart);
#elif defined(__WXGTK__)
boost::filesystem::path GetAutostartDir();
boost::filesystem::path GetAutostartFilePath();
bool GetStartOnSystemStartup();
void SetStartOnSystemStartup(bool fAutoStart);
#else
bool GetStartOnSystemStartup();
void SetStartOnSystemStartup(bool fAutoStart);
#endif

void SendingDialogOnReply2(void* parg, CDataStream& vRecv);
void SendingDialogOnReply3(void* parg, CDataStream& vRecv);
void CreateMainWindow();

#include "ui-inl.h"

#endif
