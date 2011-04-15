//////////////////////////////////////////////////////////////////////////////
//
// COptionsDialog
//

#include "COptionsDialog.h"

#include "main.h"
#include "peer.h"
#include "miner.h"
#include "util.h"
#include "CWalletDB.h"
#include "CMyTaskBarIcon.h"

COptionsDialog::COptionsDialog(wxWindow* parent) : COptionsDialogBase(parent)
{
    // Set up list box of page choices
    m_listBox->Append(_("Main"));
    //m_listBox->Append(_("Test 2"));
    m_listBox->SetSelection(0);
    SelectPage(0);
#ifndef __WXMSW__
    SetSize(1.0 * GetSize().GetWidth(), 1.2 * GetSize().GetHeight());
#endif
#if defined(__WXGTK__) || defined(__WXMAC_OSX__)
    m_checkBoxStartOnSystemStartup->SetLabel(_("&Start Bitcoin on window system startup"));
    if (!GetBoolArg("-minimizetotray"))
    {
        // Minimize to tray is just too buggy on Linux
        fMinimizeToTray = false;
        m_checkBoxMinimizeToTray->SetValue(false);
        m_checkBoxMinimizeToTray->Enable(false);
        m_checkBoxMinimizeOnClose->SetLabel(_("&Minimize on close"));
    }
#endif
#ifdef __WXMAC_OSX__
    m_checkBoxStartOnSystemStartup->Enable(false); // not implemented yet
#endif

    // Init values
    m_textCtrlTransactionFee->SetValue(FormatMoney(nTransactionFee));
    m_checkBoxLimitProcessors->SetValue(fLimitProcessors);
    m_spinCtrlLimitProcessors->Enable(fLimitProcessors);
    m_spinCtrlLimitProcessors->SetValue(nLimitProcessors);
    int nProcessors = wxThread::GetCPUCount();
    if (nProcessors < 1)
        nProcessors = 999;
    m_spinCtrlLimitProcessors->SetRange(1, nProcessors);
    m_checkBoxStartOnSystemStartup->SetValue(fTmpStartOnSystemStartup = GetStartOnSystemStartup());
    m_checkBoxMinimizeToTray->SetValue(fMinimizeToTray);
    m_checkBoxMinimizeOnClose->SetValue(fMinimizeOnClose);
    if (fHaveUPnP)
        m_checkBoxUseUPnP->SetValue(fUseUPnP);
    else
        m_checkBoxUseUPnP->Enable(false);
    m_checkBoxUseProxy->SetValue(fUseProxy);
    m_textCtrlProxyIP->Enable(fUseProxy);
    m_textCtrlProxyPort->Enable(fUseProxy);
    m_staticTextProxyIP->Enable(fUseProxy);
    m_staticTextProxyPort->Enable(fUseProxy);
    m_textCtrlProxyIP->SetValue(addrProxy.ToStringIP());
    m_textCtrlProxyPort->SetValue(addrProxy.ToStringPort());

    m_buttonOK->SetFocus();
}

void COptionsDialog::SelectPage(int nPage)
{
    m_panelMain->Show(nPage == 0);
    m_panelTest2->Show(nPage == 1);

    m_scrolledWindow->Layout();
    m_scrolledWindow->SetScrollbars(0, 0, 0, 0, 0, 0);
}

void COptionsDialog::OnListBox(wxCommandEvent& event)
{
    SelectPage(event.GetSelection());
}

void COptionsDialog::OnKillFocusTransactionFee(wxFocusEvent& event)
{
    event.Skip();
    int64 nTmp = nTransactionFee;
    ParseMoney(m_textCtrlTransactionFee->GetValue(), nTmp);
    m_textCtrlTransactionFee->SetValue(FormatMoney(nTmp));
}

void COptionsDialog::OnCheckBoxLimitProcessors(wxCommandEvent& event)
{
    m_spinCtrlLimitProcessors->Enable(event.IsChecked());
}

void COptionsDialog::OnCheckBoxUseProxy(wxCommandEvent& event)
{
    m_textCtrlProxyIP->Enable(event.IsChecked());
    m_textCtrlProxyPort->Enable(event.IsChecked());
    m_staticTextProxyIP->Enable(event.IsChecked());
    m_staticTextProxyPort->Enable(event.IsChecked());
}

CAddress COptionsDialog::GetProxyAddr()
{
    // Be careful about byte order, addr.ip and addr.port are big endian
    CAddress addr(m_textCtrlProxyIP->GetValue() + ":" + m_textCtrlProxyPort->GetValue());
    if (addr.ip == INADDR_NONE)
        addr.ip = addrProxy.ip;
    int nPort = atoi(m_textCtrlProxyPort->GetValue());
    addr.port = htons(nPort);
    if (nPort <= 0 || nPort > USHRT_MAX)
        addr.port = addrProxy.port;
    return addr;
}

void COptionsDialog::OnKillFocusProxy(wxFocusEvent& event)
{
    event.Skip();
    m_textCtrlProxyIP->SetValue(GetProxyAddr().ToStringIP());
    m_textCtrlProxyPort->SetValue(GetProxyAddr().ToStringPort());
}


void COptionsDialog::OnButtonOK(wxCommandEvent& event)
{
    OnButtonApply(event);
    EndModal(false);
}

void COptionsDialog::OnButtonCancel(wxCommandEvent& event)
{
    EndModal(false);
}

void COptionsDialog::OnButtonApply(wxCommandEvent& event)
{
    CWalletDB walletdb;

    int64 nPrevTransactionFee = nTransactionFee;
    if (ParseMoney(m_textCtrlTransactionFee->GetValue(), nTransactionFee) && nTransactionFee != nPrevTransactionFee)
        walletdb.WriteSetting("nTransactionFee", nTransactionFee);

    int nPrevMaxProc = (fLimitProcessors ? nLimitProcessors : INT_MAX);
    if (fLimitProcessors != m_checkBoxLimitProcessors->GetValue())
    {
        fLimitProcessors = m_checkBoxLimitProcessors->GetValue();
        walletdb.WriteSetting("fLimitProcessors", fLimitProcessors);
    }
    if (nLimitProcessors != m_spinCtrlLimitProcessors->GetValue())
    {
        nLimitProcessors = m_spinCtrlLimitProcessors->GetValue();
        walletdb.WriteSetting("nLimitProcessors", nLimitProcessors);
    }
    if (fGenerateBitcoins && (fLimitProcessors ? nLimitProcessors : INT_MAX) > nPrevMaxProc)
        GenerateBitcoins(fGenerateBitcoins);

    if (fTmpStartOnSystemStartup != m_checkBoxStartOnSystemStartup->GetValue())
    {
        fTmpStartOnSystemStartup = m_checkBoxStartOnSystemStartup->GetValue();
        SetStartOnSystemStartup(fTmpStartOnSystemStartup);
    }

    if (fMinimizeToTray != m_checkBoxMinimizeToTray->GetValue())
    {
        fMinimizeToTray = m_checkBoxMinimizeToTray->GetValue();
        walletdb.WriteSetting("fMinimizeToTray", fMinimizeToTray);
        ptaskbaricon->Show(fMinimizeToTray || fClosedToTray);
    }

    if (fMinimizeOnClose != m_checkBoxMinimizeOnClose->GetValue())
    {
        fMinimizeOnClose = m_checkBoxMinimizeOnClose->GetValue();
        walletdb.WriteSetting("fMinimizeOnClose", fMinimizeOnClose);
    }

    if (fHaveUPnP && fUseUPnP != m_checkBoxUseUPnP->GetValue())
    {
        fUseUPnP = m_checkBoxUseUPnP->GetValue();
        walletdb.WriteSetting("fUseUPnP", fUseUPnP);
        MapPort(fUseUPnP);
    }

    fUseProxy = m_checkBoxUseProxy->GetValue();
    walletdb.WriteSetting("fUseProxy", fUseProxy);

    addrProxy = GetProxyAddr();
    walletdb.WriteSetting("addrProxy", addrProxy);
}
