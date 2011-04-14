#ifndef CSENDINGDIALOG_H
#define CSENDINGDIALOG_H

#include "uibase.h"

#include "headers.h"
#include "util.h"
#include "CAddress.h"
#include "CWalletTx.h"

class CSendingDialog : public CSendingDialogBase
{
public:
    // Event handlers
    void OnClose(wxCloseEvent& event);
    void OnButtonOK(wxCommandEvent& event);
    void OnButtonCancel(wxCommandEvent& event);
    void OnPaint(wxPaintEvent& event);
	
public:
    /** Constructor */
    CSendingDialog(wxWindow* parent, const CAddress& addrIn, int64 nPriceIn, const CWalletTx& wtxIn);
    ~CSendingDialog();

    // State
    CAddress addr;
    int64 nPrice;
    CWalletTx wtx;
    wxDateTime start;
    char pszStatus[10000];
    bool fCanCancel;
    bool fAbort;
    bool fSuccess;
    bool fUIDone;
    bool fWorkDone;

    void Close();
    void Repaint();
    bool Status();
    bool Status(const string& str);
    bool Error(const string& str);
    void StartTransfer();
    void OnReply2(CDataStream& vRecv);
    void OnReply3(CDataStream& vRecv);
};

#endif
