#ifndef CGETTEXTFROMUSERDIALOG_H
#define CGETTEXTFROMUSERDIALOG_H

#include "uibase.h"
#include "headers.h"

class CGetTextFromUserDialog : public CGetTextFromUserDialogBase
{
protected:
    // Event handlers
    void OnButtonOK(wxCommandEvent& event);
    void OnButtonCancel(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnKeyDown(wxKeyEvent& event);

public:
    /** Constructor */
    CGetTextFromUserDialog(wxWindow* parent,
                           const string& strCaption,
                           const string& strMessage1,
                           const string& strValue1="",
                           const string& strMessage2="",
                           const string& strValue2="");

    // Custom
    string GetValue();
    string GetValue1();
    string GetValue2();
};

#endif
