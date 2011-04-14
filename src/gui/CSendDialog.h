#ifndef CSENDDIALOG_H
#define CSENDDIALOG_H

#include "uibase.h"
#include "headers.h"

class CSendDialog : public CSendDialogBase
{
protected:
    // Event handlers
    void OnKeyDown(wxKeyEvent& event);
    void OnKillFocusAmount(wxFocusEvent& event);
    void OnButtonAddressBook(wxCommandEvent& event);
    void OnButtonPaste(wxCommandEvent& event);
    void OnButtonSend(wxCommandEvent& event);
    void OnButtonCancel(wxCommandEvent& event);
	
public:
    /** Constructor */
    CSendDialog(wxWindow* parent, const wxString& strAddress="");

    // Custom
    bool fEnabledPrev;
    string strFromSave;
    string strMessageSave;
};

#endif
