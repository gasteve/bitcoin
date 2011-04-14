//////////////////////////////////////////////////////////////////////////////
//
// CSendingDialog
//

#include "CSendingDialog.h"

#include "main.h"
#include "peer.h"
#include "wallet.h"
#include "CNode.h"
#include "CCriticalBlock.h"

CSendingDialog::CSendingDialog(wxWindow* parent, const CAddress& addrIn, int64 nPriceIn, const CWalletTx& wtxIn) : CSendingDialogBase(NULL) // we have to give null so parent can't destroy us
{
    addr = addrIn;
    nPrice = nPriceIn;
    wtx = wtxIn;
    start = wxDateTime::UNow();
    memset(pszStatus, 0, sizeof(pszStatus));
    fCanCancel = true;
    fAbort = false;
    fSuccess = false;
    fUIDone = false;
    fWorkDone = false;
#ifndef __WXMSW__
    SetSize(1.2 * GetSize().GetWidth(), 1.08 * GetSize().GetHeight());
#endif

    SetTitle(strprintf(_("Sending %s to %s"), FormatMoney(nPrice).c_str(), wtx.mapValue["to"].c_str()));
    m_textCtrlStatus->SetValue("");

    CreateThread(SendingDialogStartTransfer, this);
}

CSendingDialog::~CSendingDialog()
{
    printf("~CSendingDialog()\n");
}

void CSendingDialog::Close()
{
    // Last one out turn out the lights.
    // fWorkDone signals that work side is done and UI thread should call destroy.
    // fUIDone signals that UI window has closed and work thread should call destroy.
    // This allows the window to disappear and end modality when cancelled
    // without making the user wait for ConnectNode to return.  The dialog object
    // hangs around in the background until the work thread exits.
    if (IsModal())
        EndModal(fSuccess);
    else
        Show(false);
    if (fWorkDone)
        Destroy();
    else
        fUIDone = true;
}

void CSendingDialog::OnClose(wxCloseEvent& event)
{
    if (!event.CanVeto() || fWorkDone || fAbort || !fCanCancel)
    {
        Close();
    }
    else
    {
        event.Veto();
        wxCommandEvent cmdevent;
        OnButtonCancel(cmdevent);
    }
}

void CSendingDialog::OnButtonOK(wxCommandEvent& event)
{
    if (fWorkDone)
        Close();
}

void CSendingDialog::OnButtonCancel(wxCommandEvent& event)
{
    if (fCanCancel)
        fAbort = true;
}

void CSendingDialog::OnPaint(wxPaintEvent& event)
{
    event.Skip();
    if (strlen(pszStatus) > 130)
        m_textCtrlStatus->SetValue(string("\n") + pszStatus);
    else
        m_textCtrlStatus->SetValue(string("\n\n") + pszStatus);
    m_staticTextSending->SetFocus();
    if (!fCanCancel)
        m_buttonCancel->Enable(false);
    if (fWorkDone)
    {
        m_buttonOK->Enable(true);
        m_buttonOK->SetFocus();
        m_buttonCancel->Enable(false);
    }
    if (fAbort && fCanCancel && IsShown())
    {
        strcpy(pszStatus, _("CANCELLED"));
        m_buttonOK->Enable(true);
        m_buttonOK->SetFocus();
        m_buttonCancel->Enable(false);
        m_buttonCancel->SetLabel(_("Cancelled"));
        Close();
        wxMessageBox(_("Transfer cancelled  "), _("Sending..."), wxOK, this);
    }
}


//
// Everything from here on is not in the UI thread and must only communicate
// with the rest of the dialog through variables and calling repaint.
//

void CSendingDialog::Repaint()
{
    Refresh();
    wxPaintEvent event;
    GetEventHandler()->AddPendingEvent(event);
}

bool CSendingDialog::Status()
{
    if (fUIDone)
    {
        Destroy();
        return false;
    }
    if (fAbort && fCanCancel)
    {
        memset(pszStatus, 0, 10);
        strcpy(pszStatus, _("CANCELLED"));
        Repaint();
        fWorkDone = true;
        return false;
    }
    return true;
}

bool CSendingDialog::Status(const string& str)
{
    if (!Status())
        return false;

    // This can be read by the UI thread at any time,
    // so copy in a way that can be read cleanly at all times.
    memset(pszStatus, 0, min(str.size()+1, sizeof(pszStatus)));
    strlcpy(pszStatus, str.c_str(), sizeof(pszStatus));

    Repaint();
    return true;
}

bool CSendingDialog::Error(const string& str)
{
    fCanCancel = false;
    fWorkDone = true;
    Status(string(_("Error: ")) + str);
    return false;
}

void SendingDialogStartTransfer(void* parg)
{
    ((CSendingDialog*)parg)->StartTransfer();
}

void CSendingDialog::StartTransfer()
{
    // Make sure we have enough money
    if (nPrice + nTransactionFee > GetBalance())
    {
        Error(_("Insufficient funds"));
        return;
    }

    // We may have connected already for product details
    if (!Status(_("Connecting...")))
        return;
    CNode* pnode = ConnectNode(addr, 15 * 60);
    if (!pnode)
    {
        Error(_("Unable to connect"));
        return;
    }

    // Send order to seller, with response going to OnReply2 via event handler
    if (!Status(_("Requesting public key...")))
        return;
    pnode->PushRequest("checkorder", wtx, SendingDialogOnReply2, this);
}

void CSendingDialog::OnReply2(CDataStream& vRecv)
{
    if (!Status(_("Received public key...")))
        return;

    CScript scriptPubKey;
    int nRet;
    try
    {
        vRecv >> nRet;
        if (nRet > 0)
        {
            string strMessage;
            if (!vRecv.empty())
                vRecv >> strMessage;
            if (nRet == 2)
                Error(_("Recipient is not accepting transactions sent by IP address"));
            else
                Error(_("Transfer was not accepted"));
            //// todo: enlarge the window and enable a hidden white box to put seller's message
            return;
        }
        vRecv >> scriptPubKey;
    }
    catch (...)
    {
        //// what do we want to do about this?
        Error(_("Invalid response received"));
        return;
    }

    // Pause to give the user a chance to cancel
    while (wxDateTime::UNow() < start + wxTimeSpan(0, 0, 0, 2 * 1000))
    {
        Sleep(200);
        if (!Status())
            return;
    }

    CRITICAL_BLOCK(cs_main)
    {
        // Pay
        if (!Status(_("Creating transaction...")))
            return;
        if (nPrice + nTransactionFee > GetBalance())
        {
            Error(_("Insufficient funds"));
            return;
        }
        CReserveKey reservekey;
        int64 nFeeRequired;
        if (!CreateTransaction(scriptPubKey, nPrice, wtx, reservekey, nFeeRequired))
        {
            if (nPrice + nFeeRequired > GetBalance())
                Error(strprintf(_("This is an oversized transaction that requires a transaction fee of %s"), FormatMoney(nFeeRequired).c_str()));
            else
                Error(_("Transaction creation failed"));
            return;
        }

        // Transaction fee
        if (!ThreadSafeAskFee(nFeeRequired, _("Sending..."), this))
        {
            Error(_("Transaction aborted"));
            return;
        }

        // Make sure we're still connected
        CNode* pnode = ConnectNode(addr, 2 * 60 * 60);
        if (!pnode)
        {
            Error(_("Lost connection, transaction cancelled"));
            return;
        }

        // Last chance to cancel
        Sleep(50);
        if (!Status())
            return;
        fCanCancel = false;
        if (fAbort)
        {
            fCanCancel = true;
            if (!Status())
                return;
            fCanCancel = false;
        }
        if (!Status(_("Sending payment...")))
            return;

        // Commit
        if (!CommitTransaction(wtx, reservekey))
        {
            Error(_("The transaction was rejected.  This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here."));
            return;
        }

        // Send payment tx to seller, with response going to OnReply3 via event handler
        CWalletTx wtxSend = wtx;
        wtxSend.fFromMe = false;
        pnode->PushRequest("submitorder", wtxSend, SendingDialogOnReply3, this);

        Status(_("Waiting for confirmation..."));
        MainFrameRepaint();
    }
}

void CSendingDialog::OnReply3(CDataStream& vRecv)
{
    int nRet;
    try
    {
        vRecv >> nRet;
        if (nRet > 0)
        {
            Error(_("The payment was sent, but the recipient was unable to verify it.\n"
                    "The transaction is recorded and will credit to the recipient,\n"
                    "but the comment information will be blank."));
            return;
        }
    }
    catch (...)
    {
        //// what do we want to do about this?
        Error(_("Payment was sent, but an invalid response was received"));
        return;
    }

    fSuccess = true;
    fWorkDone = true;
    Status(_("Payment completed"));
}
