#ifndef COPTIONSDIALOG_H
#define COPTIONSDIALOG_H

#include "uibase.h"
#include "headers.h"

class CAddress;

class COptionsDialog : public COptionsDialogBase
{
protected:
    // Event handlers
    void OnListBox(wxCommandEvent& event);
    void OnKillFocusTransactionFee(wxFocusEvent& event);
    void OnCheckBoxLimitProcessors(wxCommandEvent& event);
    void OnCheckBoxUseProxy(wxCommandEvent& event);
    void OnKillFocusProxy(wxFocusEvent& event);

    void OnButtonOK(wxCommandEvent& event);
    void OnButtonCancel(wxCommandEvent& event);
    void OnButtonApply(wxCommandEvent& event);

public:
    /** Constructor */
    COptionsDialog(wxWindow* parent);

    // Custom
    bool fTmpStartOnSystemStartup;
    bool fTmpMinimizeOnClose;
    void SelectPage(int nPage);
    CAddress GetProxyAddr();
};

#endif
