#ifndef CTXDETAILSDIALOG_H
#define CTXDETAILSDIALOG_H

#include "uibase.h"
#include "CWalletTx.h"

class CTxDetailsDialog : public CTxDetailsDialogBase
{
protected:
    // Event handlers
    void OnButtonOK(wxCommandEvent& event);

public:
    /** Constructor */
    CTxDetailsDialog(wxWindow* parent, CWalletTx wtx);
    
    // State
    CWalletTx wtx;
};

#endif
