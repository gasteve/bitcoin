#ifndef CMYTASKBARICON_H
#define CMYTASKBARICON_H

#include "headers.h"

class CMyTaskBarIcon : public wxTaskBarIcon
{
protected:
    // Event handlers
    void OnLeftButtonDClick(wxTaskBarIconEvent& event);
    void OnMenuRestore(wxCommandEvent& event);
    void OnMenuOptions(wxCommandEvent& event);
    void OnUpdateUIGenerate(wxUpdateUIEvent& event);
    void OnMenuGenerate(wxCommandEvent& event);
    void OnMenuExit(wxCommandEvent& event);

public:
    CMyTaskBarIcon();
    void Show(bool fShow=true);
    void Hide();
    void Restore();
    void UpdateTooltip();
    virtual wxMenu* CreatePopupMenu();

DECLARE_EVENT_TABLE()
};

#endif
