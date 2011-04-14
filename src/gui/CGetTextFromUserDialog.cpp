#include "CGetTextFromUserDialog.h"

#include "headers.h"
#include "util.h"
#include "ui.h"
#include "uibase.h"

// Event handlers
void CGetTextFromUserDialog::OnButtonOK(wxCommandEvent& event)     { EndModal(true); }
void CGetTextFromUserDialog::OnButtonCancel(wxCommandEvent& event) { EndModal(false); }
void CGetTextFromUserDialog::OnClose(wxCloseEvent& event)          { EndModal(false); }

void CGetTextFromUserDialog::OnKeyDown(wxKeyEvent& event)
{
    if (event.GetKeyCode() == '\r' || event.GetKeyCode() == WXK_NUMPAD_ENTER)
        EndModal(true);
    else
        HandleCtrlA(event);
}

/** Constructor */
CGetTextFromUserDialog::CGetTextFromUserDialog(wxWindow* parent,
                       const string& strCaption,
                       const string& strMessage1,
                       const string& strValue1,
                       const string& strMessage2,
                       const string& strValue2) : CGetTextFromUserDialogBase(parent, wxID_ANY, strCaption)
{
    int x = GetSize().GetWidth();
    int y = GetSize().GetHeight();
    m_staticTextMessage1->SetLabel(strMessage1);
    m_textCtrl1->SetValue(strValue1);
    y += wxString(strMessage1).Freq('\n') * 14;
    if (!strMessage2.empty())
    {
        m_staticTextMessage2->Show(true);
        m_staticTextMessage2->SetLabel(strMessage2);
        m_textCtrl2->Show(true);
        m_textCtrl2->SetValue(strValue2);
        y += 46 + wxString(strMessage2).Freq('\n') * 14;
    }
#ifndef __WXMSW__
    x = x * 114 / 100;
    y = y * 114 / 100;
#endif
    SetSize(x, y);
}

// Custom
string CGetTextFromUserDialog::GetValue()  { return (string)m_textCtrl1->GetValue(); }
string CGetTextFromUserDialog::GetValue1() { return (string)m_textCtrl1->GetValue(); }
string CGetTextFromUserDialog::GetValue2() { return (string)m_textCtrl2->GetValue(); }

