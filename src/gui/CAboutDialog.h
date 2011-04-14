#ifndef CABOUTDIALOG_H
#define CABOUTDIALOG_H

#include "uibase.h"

class CAboutDialog : public CAboutDialogBase
{
protected:
    // Event handlers
    void OnButtonOK(wxCommandEvent& event);

public:
    /** Constructor */
    CAboutDialog(wxWindow* parent);
};

#endif
