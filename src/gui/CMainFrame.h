#ifndef CMAINFRAME_H
#define CMAINFRAME_H

#include "uibase.h"
#include "uint256.h"

class CWalletTx;

class CMainFrame : public CMainFrameBase
{
protected:
    // Event handlers
    void OnNotebookPageChanged(wxNotebookEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnIconize(wxIconizeEvent& event);
    void OnMouseEvents(wxMouseEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnIdle(wxIdleEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnPaintListCtrl(wxPaintEvent& event);
    void OnMenuFileExit(wxCommandEvent& event);
    void OnMenuOptionsGenerate(wxCommandEvent& event);
    void OnUpdateUIOptionsGenerate(wxUpdateUIEvent& event);
    void OnMenuOptionsChangeYourAddress(wxCommandEvent& event);
    void OnMenuOptionsOptions(wxCommandEvent& event);
    void OnMenuHelpAbout(wxCommandEvent& event);
    void OnButtonSend(wxCommandEvent& event);
    void OnButtonAddressBook(wxCommandEvent& event);
    void OnSetFocusAddress(wxFocusEvent& event);
    void OnMouseEventsAddress(wxMouseEvent& event);
    void OnButtonNew(wxCommandEvent& event);
    void OnButtonCopy(wxCommandEvent& event);
    void OnListColBeginDrag(wxListEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnListItemActivatedProductsSent(wxListEvent& event);
    void OnListItemActivatedOrdersSent(wxListEvent& event);
    void OnListItemActivatedOrdersReceived(wxListEvent& event);
	
public:
    /** Constructor */
    CMainFrame(wxWindow* parent);
    ~CMainFrame();

    // Custom
    enum
    {
        ALL = 0,
        SENTRECEIVED = 1,
        SENT = 2,
        RECEIVED = 3,
    };
    int nPage;
    wxListCtrl* m_listCtrl;
    bool fShowGenerated;
    bool fShowSent;
    bool fShowReceived;
    bool fRefreshListCtrl;
    bool fRefreshListCtrlRunning;
    bool fOnSetFocusAddress;
    unsigned int nListViewUpdated;
    bool fRefresh;

    void OnUIThreadCall(wxCommandEvent& event);
    int GetSortIndex(const string& strSort);
    void InsertLine(bool fNew, int nIndex, uint256 hashKey, string strSort, const wxColour& colour, const wxString& str1, const wxString& str2, const wxString& str3, const wxString& str4, const wxString& str5);
    bool DeleteLine(uint256 hashKey);
    bool InsertTransaction(const CWalletTx& wtx, bool fNew, int nIndex=-1);
    void RefreshListCtrl();
    void RefreshStatusColumn();
};

#endif
