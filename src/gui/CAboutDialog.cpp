//////////////////////////////////////////////////////////////////////////////
//
// CAboutDialog
//

#include "CAboutDialog.h"
#include "util.h"
#include "serialize.h"

CAboutDialog::CAboutDialog(wxWindow* parent) : CAboutDialogBase(parent)
{
    m_staticTextVersion->SetLabel(strprintf(_("version %s%s BETA"), FormatVersion(VERSION).c_str(), pszSubVer));

    // Change (c) into UTF-8 or ANSI copyright symbol
    wxString str = m_staticTextMain->GetLabel();
#if wxUSE_UNICODE
    str.Replace("(c)", wxString::FromUTF8("\xC2\xA9"));
#else
    str.Replace("(c)", "\xA9");
#endif
    m_staticTextMain->SetLabel(str);
#ifndef __WXMSW__
    // Resize on Linux to make the window fit the text.
    // The text was wrapped manually rather than using the Wrap setting because
    // the wrap would be too small on Linux and it can't be changed at this point.
    wxFont fontTmp = m_staticTextMain->GetFont();
    if (fontTmp.GetPointSize() > 8);
        fontTmp.SetPointSize(8);
    m_staticTextMain->SetFont(fontTmp);
    SetSize(GetSize().GetWidth() + 44, GetSize().GetHeight() + 10);
#endif
}

void CAboutDialog::OnButtonOK(wxCommandEvent& event)
{
    EndModal(false);
}
