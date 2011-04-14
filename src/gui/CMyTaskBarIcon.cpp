//////////////////////////////////////////////////////////////////////////////
//
// CMyTaskBarIcon
//

#include "CMyTaskBarIcon.h"
    
#include "headers.h"
#include "ui.h"
#include "main.h"
#include "miner.h"
#include "peer.h"

enum
{
    ID_TASKBAR_RESTORE = 10001,
    ID_TASKBAR_OPTIONS,
    ID_TASKBAR_GENERATE,
    ID_TASKBAR_EXIT,
};

BEGIN_EVENT_TABLE(CMyTaskBarIcon, wxTaskBarIcon)
    EVT_TASKBAR_LEFT_DCLICK(CMyTaskBarIcon::OnLeftButtonDClick)
    EVT_MENU(ID_TASKBAR_RESTORE, CMyTaskBarIcon::OnMenuRestore)
    EVT_MENU(ID_TASKBAR_OPTIONS, CMyTaskBarIcon::OnMenuOptions)
    EVT_MENU(ID_TASKBAR_GENERATE, CMyTaskBarIcon::OnMenuGenerate)
    EVT_UPDATE_UI(ID_TASKBAR_GENERATE, CMyTaskBarIcon::OnUpdateUIGenerate)
    EVT_MENU(ID_TASKBAR_EXIT, CMyTaskBarIcon::OnMenuExit)
END_EVENT_TABLE()

CMyTaskBarIcon::CMyTaskBarIcon() : wxTaskBarIcon()
{
    Show(true);
}

void CMyTaskBarIcon::Show(bool fShow)
{
    static char pszPrevTip[200];
    if (fShow)
    {
        string strTooltip = _("Bitcoin");
        if (fGenerateBitcoins)
            strTooltip = _("Bitcoin - Generating");
        if (fGenerateBitcoins && vNodes.empty())
            strTooltip = _("Bitcoin - (not connected)");

        // Optimization, only update when changed, using char array to be reentrant
        if (strncmp(pszPrevTip, strTooltip.c_str(), sizeof(pszPrevTip)-1) != 0)
        {
            strlcpy(pszPrevTip, strTooltip.c_str(), sizeof(pszPrevTip));
#ifdef __WXMSW__
            // somehow it'll choose the wrong size and scale it down if
            // we use the main icon, so we hand it one with only 16x16
            SetIcon(wxICON(favicon), strTooltip);
#else
            SetIcon(bitcoin80_xpm, strTooltip);
#endif
        }
    }
    else
    {
        strlcpy(pszPrevTip, "", sizeof(pszPrevTip));
        RemoveIcon();
    }
}

void CMyTaskBarIcon::Hide()
{
    Show(false);
}

void CMyTaskBarIcon::OnLeftButtonDClick(wxTaskBarIconEvent& event)
{
    Restore();
}

void CMyTaskBarIcon::OnMenuRestore(wxCommandEvent& event)
{
    Restore();
}

void CMyTaskBarIcon::OnMenuOptions(wxCommandEvent& event)
{
    // Since it's modal, get the main window to do it
    wxCommandEvent event2(wxEVT_COMMAND_MENU_SELECTED, wxID_PREFERENCES);
    pframeMain->GetEventHandler()->AddPendingEvent(event2);
}

void CMyTaskBarIcon::Restore()
{
    pframeMain->Show();
    wxIconizeEvent event(0, false);
    pframeMain->GetEventHandler()->AddPendingEvent(event);
    pframeMain->Iconize(false);
    pframeMain->Raise();
}

void CMyTaskBarIcon::OnMenuGenerate(wxCommandEvent& event)
{
    GenerateBitcoins(event.IsChecked());
}

void CMyTaskBarIcon::OnUpdateUIGenerate(wxUpdateUIEvent& event)
{
    event.Check(fGenerateBitcoins);
}

void CMyTaskBarIcon::OnMenuExit(wxCommandEvent& event)
{
    pframeMain->Close(true);
}

void CMyTaskBarIcon::UpdateTooltip()
{
    if (IsIconInstalled())
        Show(true);
}

wxMenu* CMyTaskBarIcon::CreatePopupMenu()
{
    wxMenu* pmenu = new wxMenu;
    pmenu->Append(ID_TASKBAR_RESTORE, _("&Open Bitcoin"));
    pmenu->Append(ID_TASKBAR_OPTIONS, _("O&ptions..."));
    pmenu->AppendCheckItem(ID_TASKBAR_GENERATE, _("&Generate Coins"))->Check(fGenerateBitcoins);
#ifndef __WXMAC_OSX__ // Mac has built-in quit menu
    pmenu->AppendSeparator();
    pmenu->Append(ID_TASKBAR_EXIT, _("E&xit"));
#endif
    return pmenu;
}
