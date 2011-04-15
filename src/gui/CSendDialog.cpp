//////////////////////////////////////////////////////////////////////////////
//
// CSendDialog
//

#include "CSendDialog.h"

#include "main.h"
#include "base58.h"
#include "ui.h"
#include "wallet.h"
#include "CAddressBookDialog.h"
#include "CCriticalSection.h"
#include "CTryCriticalBlock.h"
#include "CWalletTx.h"
#include "CAddress.h"
#include "CSendingDialog.h"

CSendDialog::CSendDialog(wxWindow* parent, const wxString& strAddress) : CSendDialogBase(parent)
{
    // Init
    m_textCtrlAddress->SetValue(strAddress);
    m_choiceTransferType->SetSelection(0);
    m_bitmapCheckMark->Show(false);
    fEnabledPrev = true;
    m_textCtrlAddress->SetFocus();
    //// todo: should add a display of your balance for convenience
#ifndef __WXMSW__
    wxFont fontTmp = m_staticTextInstructions->GetFont();
    if (fontTmp.GetPointSize() > 9);
        fontTmp.SetPointSize(9);
    m_staticTextInstructions->SetFont(fontTmp);
    SetSize(725, 180);
#endif

    // Set Icon
    wxIcon iconSend;
    iconSend.CopyFromBitmap(wxBitmap(send16noshadow_xpm));
    SetIcon(iconSend);

    // Fixup the tab order
    m_buttonPaste->MoveAfterInTabOrder(m_buttonCancel);
    m_buttonAddress->MoveAfterInTabOrder(m_buttonPaste);
    this->Layout();
}

void CSendDialog::OnKeyDown(wxKeyEvent& event) 
{ 
    HandleCtrlA(event); 
}

void CSendDialog::OnKillFocusAmount(wxFocusEvent& event)
{
    // Reformat the amount
    event.Skip();
    if (m_textCtrlAmount->GetValue().Trim().empty())
        return;
    int64 nTmp;
    if (ParseMoney(m_textCtrlAmount->GetValue(), nTmp))
        m_textCtrlAmount->SetValue(FormatMoney(nTmp));
}

void CSendDialog::OnButtonAddressBook(wxCommandEvent& event)
{
    // Open address book
    CAddressBookDialog dialog(this, m_textCtrlAddress->GetValue(), CAddressBookDialog::SENDING, true);
    if (dialog.ShowModal())
        m_textCtrlAddress->SetValue(dialog.GetSelectedAddress());
}

void CSendDialog::OnButtonPaste(wxCommandEvent& event)
{
    // Copy clipboard to address box
    if (wxTheClipboard->Open())
    {
        if (wxTheClipboard->IsSupported(wxDF_TEXT))
        {
            wxTextDataObject data;
            wxTheClipboard->GetData(data);
            m_textCtrlAddress->SetValue(data.GetText());
        }
        wxTheClipboard->Close();
    }
}

void CSendDialog::OnButtonSend(wxCommandEvent& event)
{
    static CCriticalSection cs_sendlock;
    TRY_CRITICAL_BLOCK(cs_sendlock)
    {
        CWalletTx wtx;
        string strAddress = (string)m_textCtrlAddress->GetValue();

        // Parse amount
        int64 nValue = 0;
        if (!ParseMoney(m_textCtrlAmount->GetValue(), nValue) || nValue <= 0)
        {
            wxMessageBox(_("Error in amount  "), _("Send Coins"));
            return;
        }
        if (nValue > GetBalance())
        {
            wxMessageBox(_("Amount exceeds your balance  "), _("Send Coins"));
            return;
        }
        if (nValue + nTransactionFee > GetBalance())
        {
            wxMessageBox(string(_("Total exceeds your balance when the ")) + FormatMoney(nTransactionFee) + _(" transaction fee is included  "), _("Send Coins"));
            return;
        }

        // Parse bitcoin address
        uint160 hash160;
        bool fBitcoinAddress = AddressToHash160(strAddress, hash160);

        if (fBitcoinAddress)
        {
	    CRITICAL_BLOCK(cs_main)
	    {
                // Send to bitcoin address
                CScript scriptPubKey;
                scriptPubKey << OP_DUP << OP_HASH160 << hash160 << OP_EQUALVERIFY << OP_CHECKSIG;

                string strError = SendMoney(scriptPubKey, nValue, wtx, true);
                if (strError == "")
                    wxMessageBox(_("Payment sent  "), _("Sending..."));
                else if (strError == "ABORTED")
                    return; // leave send dialog open
                else
                {
                    wxMessageBox(strError + "  ", _("Sending..."));
                    EndModal(false);
                }
	    }
        }
        else
        {
            // Parse IP address
            CAddress addr(strAddress);
            if (!addr.IsValid())
            {
                wxMessageBox(_("Invalid address  "), _("Send Coins"));
                return;
            }

            // Message
            wtx.mapValue["to"] = strAddress;

            // Send to IP address
            CSendingDialog* pdialog = new CSendingDialog(this, addr, nValue, wtx);
            if (!pdialog->ShowModal())
                return;
        }

        CRITICAL_BLOCK(cs_mapAddressBook)
            if (!mapAddressBook.count(strAddress))
                SetAddressBookName(strAddress, "");

        EndModal(true);
    }
}

void CSendDialog::OnButtonCancel(wxCommandEvent& event)
{
    // Cancel
    EndModal(false);
}
