#ifndef CADDRESSBOOKDIALOG_H
#define CADDRESSBOOKDIALOG_H

#include "headers.h"
#include "uibase.h"

class CAddressBookDialog : public CAddressBookDialogBase
{
protected:
    // Event handlers
    void OnNotebookPageChanged(wxNotebookEvent& event);
    void OnListEndLabelEdit(wxListEvent& event);
    void OnListItemSelected(wxListEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnButtonDelete(wxCommandEvent& event);
    void OnButtonCopy(wxCommandEvent& event);
    void OnButtonEdit(wxCommandEvent& event);
    void OnButtonNew(wxCommandEvent& event);
    void OnButtonOK(wxCommandEvent& event);
    void OnButtonCancel(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

public:
    /** Constructor */
    CAddressBookDialog(wxWindow* parent, const wxString& strInitSelected, int nPageIn, bool fDuringSendIn);

    // Custom
    enum
    {
        SENDING = 0,
        RECEIVING = 1,
    };
    int nPage;
    wxListCtrl* m_listCtrl;
    bool fDuringSend;
    wxString GetAddress();
    wxString GetSelectedAddress();
    wxString GetSelectedSendingAddress();
    wxString GetSelectedReceivingAddress();
    bool CheckIfMine(const string& strAddress, const string& strTitle);
};

#endif
