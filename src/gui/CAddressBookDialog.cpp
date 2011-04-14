//////////////////////////////////////////////////////////////////////////////
//
// CAddressBookDialog
//

#include "CAddressBookDialog.h"
#include "main.h"
#include "util.h"
#include "headers.h"
#include "ui.h"
#include "base58.h"
#include "db.h"
#include "CCriticalBlock.h"
#include "CWalletDB.h"
#include "CGetTextFromuserDialog.h"

CAddressBookDialog::CAddressBookDialog(wxWindow* parent, const wxString& strInitSelected, int nPageIn, bool fDuringSendIn) : CAddressBookDialogBase(parent)
{
    // Set initially selected page
    wxNotebookEvent event;
    event.SetSelection(nPageIn);
    OnNotebookPageChanged(event);
    m_notebook->ChangeSelection(nPageIn);

    fDuringSend = fDuringSendIn;
    if (!fDuringSend)
        m_buttonCancel->Show(false);

    // Set Icon
    wxIcon iconAddressBook;
    iconAddressBook.CopyFromBitmap(wxBitmap(addressbook16_xpm));
    SetIcon(iconAddressBook);

    // Init column headers
    m_listCtrlSending->InsertColumn(0, _("Name"), wxLIST_FORMAT_LEFT, 200);
    m_listCtrlSending->InsertColumn(1, _("Address"), wxLIST_FORMAT_LEFT, 350);
    m_listCtrlSending->SetFocus();
    m_listCtrlReceiving->InsertColumn(0, _("Label"), wxLIST_FORMAT_LEFT, 200);
    m_listCtrlReceiving->InsertColumn(1, _("Bitcoin Address"), wxLIST_FORMAT_LEFT, 350);
    m_listCtrlReceiving->SetFocus();

    // Fill listctrl with address book data
    CRITICAL_BLOCK(cs_mapKeys)
    CRITICAL_BLOCK(cs_mapAddressBook)
    {
        string strDefaultReceiving = (string)pframeMain->m_textCtrlAddress->GetValue();
        foreach(const PAIRTYPE(string, string)& item, mapAddressBook)
        {
            string strAddress = item.first;
            string strName = item.second;
            uint160 hash160;
            bool fMine = (AddressToHash160(strAddress, hash160) && mapPubKeys.count(hash160));
            wxListCtrl* plistCtrl = fMine ? m_listCtrlReceiving : m_listCtrlSending;
            int nIndex = InsertLine(plistCtrl, strName, strAddress);
            if (strAddress == (fMine ? strDefaultReceiving : string(strInitSelected)))
                plistCtrl->SetItemState(nIndex, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
        }
    }
}

wxString CAddressBookDialog::GetSelectedAddress()
{
    int nIndex = GetSelection(m_listCtrl);
    if (nIndex == -1)
        return "";
    return GetItemText(m_listCtrl, nIndex, 1);
}

wxString CAddressBookDialog::GetSelectedSendingAddress()
{
    int nIndex = GetSelection(m_listCtrlSending);
    if (nIndex == -1)
        return "";
    return GetItemText(m_listCtrlSending, nIndex, 1);
}

wxString CAddressBookDialog::GetSelectedReceivingAddress()
{
    int nIndex = GetSelection(m_listCtrlReceiving);
    if (nIndex == -1)
        return "";
    return GetItemText(m_listCtrlReceiving, nIndex, 1);
}

void CAddressBookDialog::OnNotebookPageChanged(wxNotebookEvent& event)
{
    event.Skip();
    nPage = event.GetSelection();
    if (nPage == SENDING)
        m_listCtrl = m_listCtrlSending;
    else if (nPage == RECEIVING)
        m_listCtrl = m_listCtrlReceiving;
    m_buttonDelete->Show(nPage == SENDING);
    m_buttonCopy->Show(nPage == RECEIVING);
    this->Layout();
    m_listCtrl->SetFocus();
}

void CAddressBookDialog::OnListEndLabelEdit(wxListEvent& event)
{
    // Update address book with edited name
    event.Skip();
    if (event.IsEditCancelled())
        return;
    string strAddress = (string)GetItemText(m_listCtrl, event.GetIndex(), 1);
    SetAddressBookName(strAddress, string(event.GetText()));
    pframeMain->RefreshListCtrl();
}

void CAddressBookDialog::OnListItemSelected(wxListEvent& event)
{
    event.Skip();
    if (nPage == RECEIVING)
        SetDefaultReceivingAddress((string)GetSelectedReceivingAddress());
}

void CAddressBookDialog::OnListItemActivated(wxListEvent& event)
{
    event.Skip();
    if (fDuringSend)
    {
        // Doubleclick returns selection
        EndModal(GetSelectedAddress() != "" ? 2 : 0);
        return;
    }

    // Doubleclick edits item
    wxCommandEvent event2;
    OnButtonEdit(event2);
}

void CAddressBookDialog::OnButtonDelete(wxCommandEvent& event)
{
    if (nPage != SENDING)
        return;
    for (int nIndex = m_listCtrl->GetItemCount()-1; nIndex >= 0; nIndex--)
    {
        if (m_listCtrl->GetItemState(nIndex, wxLIST_STATE_SELECTED))
        {
            string strAddress = (string)GetItemText(m_listCtrl, nIndex, 1);
            CWalletDB().EraseName(strAddress);
            m_listCtrl->DeleteItem(nIndex);
        }
    }
    pframeMain->RefreshListCtrl();
}

void CAddressBookDialog::OnButtonCopy(wxCommandEvent& event)
{
    // Copy address box to clipboard
    if (wxTheClipboard->Open())
    {
        wxTheClipboard->SetData(new wxTextDataObject(GetSelectedAddress()));
        wxTheClipboard->Close();
    }
}

bool CAddressBookDialog::CheckIfMine(const string& strAddress, const string& strTitle)
{
    uint160 hash160;
    bool fMine = (AddressToHash160(strAddress, hash160) && mapPubKeys.count(hash160));
    if (fMine)
        wxMessageBox(_("This is one of your own addresses for receiving payments and cannot be entered in the address book.  "), strTitle);
    return fMine;
}

void CAddressBookDialog::OnButtonEdit(wxCommandEvent& event)
{
    int nIndex = GetSelection(m_listCtrl);
    if (nIndex == -1)
        return;
    string strName = (string)m_listCtrl->GetItemText(nIndex);
    string strAddress = (string)GetItemText(m_listCtrl, nIndex, 1);
    string strAddressOrg = strAddress;

    if (nPage == SENDING)
    {
        // Ask name and address
        do
        {
            CGetTextFromUserDialog dialog(this, _("Edit Address"), _("Name"), strName, _("Address"), strAddress);
            if (!dialog.ShowModal())
                return;
            strName = dialog.GetValue1();
            strAddress = dialog.GetValue2();
        }
        while (CheckIfMine(strAddress, _("Edit Address")));

    }
    else if (nPage == RECEIVING)
    {
        // Ask name
        CGetTextFromUserDialog dialog(this, _("Edit Address Label"), _("Label"), strName);
        if (!dialog.ShowModal())
            return;
        strName = dialog.GetValue();
    }

    // Write back
    if (strAddress != strAddressOrg)
        CWalletDB().EraseName(strAddressOrg);
    SetAddressBookName(strAddress, strName);
    m_listCtrl->SetItem(nIndex, 1, strAddress);
    m_listCtrl->SetItemText(nIndex, strName);
    pframeMain->RefreshListCtrl();
}

void CAddressBookDialog::OnButtonNew(wxCommandEvent& event)
{
    string strName;
    string strAddress;

    if (nPage == SENDING)
    {
        // Ask name and address
        do
        {
            CGetTextFromUserDialog dialog(this, _("Add Address"), _("Name"), strName, _("Address"), strAddress);
            if (!dialog.ShowModal())
                return;
            strName = dialog.GetValue1();
            strAddress = dialog.GetValue2();
        }
        while (CheckIfMine(strAddress, _("Add Address")));
    }
    else if (nPage == RECEIVING)
    {
        // Ask name
        CGetTextFromUserDialog dialog(this,
            _("New Receiving Address"),
            _("You should use a new address for each payment you receive.\n\nLabel"),
            "");
        if (!dialog.ShowModal())
            return;
        strName = dialog.GetValue();

        // Generate new key
        strAddress = PubKeyToAddress(GetKeyFromKeyPool());
    }

    // Add to list and select it
    SetAddressBookName(strAddress, strName);
    int nIndex = InsertLine(m_listCtrl, strName, strAddress);
    SetSelection(m_listCtrl, nIndex);
    m_listCtrl->SetFocus();
    if (nPage == SENDING)
        pframeMain->RefreshListCtrl();
}

void CAddressBookDialog::OnButtonOK(wxCommandEvent& event)
{
    // OK
    EndModal(GetSelectedAddress() != "" ? 1 : 0);
}

void CAddressBookDialog::OnButtonCancel(wxCommandEvent& event)
{
    // Cancel
    EndModal(0);
}

void CAddressBookDialog::OnClose(wxCloseEvent& event)
{
    // Close
    EndModal(0);
}
