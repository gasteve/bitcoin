//////////////////////////////////////////////////////////////////////////////
//
// CMainFrame
//

#include "CMainFrame.h"
#include "headers.h"
#include "main.h"
#include "util.h"
#include "init.h"
#include "script.h"
#include "peer.h"
#include "miner.h"
#include "uibase.h"
#include "CWalletTx.h"
#include "uint256.h"
#include "base58.h"
#include "wallet.h"
#include "CTryCriticalBlock.h"
#include "CMyTaskBarIcon.h"
#include "CAddressBookDialog.h"
#include "COptionsDialog.h"
#include "CAboutDialog.h"
#include "CSendDialog.h"
#include "CGetTextFromUserDialog.h"
#include "CTxDetailsDialog.h"

CMainFrame::CMainFrame(wxWindow* parent) : CMainFrameBase(parent)
{
    Connect(wxEVT_UITHREADCALL, wxCommandEventHandler(CMainFrame::OnUIThreadCall), NULL, this);

    // Set initially selected page
    wxNotebookEvent event;
    event.SetSelection(0);
    OnNotebookPageChanged(event);
    m_notebook->ChangeSelection(0);

    // Init
    fRefreshListCtrl = false;
    fRefreshListCtrlRunning = false;
    fOnSetFocusAddress = false;
    fRefresh = false;
    m_choiceFilter->SetSelection(0);
    double dResize = 1.0;
#ifdef __WXMSW__
    SetIcon(wxICON(bitcoin));
#else
    SetIcon(bitcoin80_xpm);
    SetBackgroundColour(m_toolBar->GetBackgroundColour());
    wxFont fontTmp = m_staticText41->GetFont();
    fontTmp.SetFamily(wxFONTFAMILY_TELETYPE);
    m_staticTextBalance->SetFont(fontTmp);
    m_staticTextBalance->SetSize(140, 17);
    // resize to fit ubuntu's huge default font
    dResize = 1.22;
    SetSize(dResize * GetSize().GetWidth(), 1.15 * GetSize().GetHeight());
#endif
    m_staticTextBalance->SetLabel(FormatMoney(GetBalance()) + "  ");
    m_listCtrl->SetFocus();
    ptaskbaricon = new CMyTaskBarIcon();
#ifdef __WXMAC_OSX__
    // Mac automatically moves wxID_EXIT, wxID_PREFERENCES and wxID_ABOUT
    // to their standard places, leaving these menus empty.
    GetMenuBar()->Remove(2); // remove Help menu
    GetMenuBar()->Remove(0); // remove File menu
#endif

    // Init column headers
    int nDateWidth = DateTimeStr(1229413914).size() * 6 + 8;
    if (!strstr(DateTimeStr(1229413914).c_str(), "2008"))
        nDateWidth += 12;
#ifdef __WXMAC_OSX__
    nDateWidth += 5;
    dResize -= 0.01;
#endif
    wxListCtrl* pplistCtrl[] = {m_listCtrlAll, m_listCtrlSentReceived, m_listCtrlSent, m_listCtrlReceived};
    foreach(wxListCtrl* p, pplistCtrl)
    {
        p->InsertColumn(0, "",               wxLIST_FORMAT_LEFT,  dResize * 0);
        p->InsertColumn(1, "",               wxLIST_FORMAT_LEFT,  dResize * 0);
        p->InsertColumn(2, _("Status"),      wxLIST_FORMAT_LEFT,  dResize * 112);
        p->InsertColumn(3, _("Date"),        wxLIST_FORMAT_LEFT,  dResize * nDateWidth);
        p->InsertColumn(4, _("Description"), wxLIST_FORMAT_LEFT,  dResize * 409 - nDateWidth);
        p->InsertColumn(5, _("Debit"),       wxLIST_FORMAT_RIGHT, dResize * 79);
        p->InsertColumn(6, _("Credit"),      wxLIST_FORMAT_RIGHT, dResize * 79);
    }

    // Init status bar
    int pnWidths[3] = { -100, 88, 300 };
#ifndef __WXMSW__
    pnWidths[1] = pnWidths[1] * 1.1 * dResize;
    pnWidths[2] = pnWidths[2] * 1.1 * dResize;
#endif
    m_statusBar->SetFieldsCount(3, pnWidths);

    // Fill your address text box
    vector<unsigned char> vchPubKey;
    if (CWalletDB("r").ReadDefaultKey(vchPubKey))
        m_textCtrlAddress->SetValue(PubKeyToAddress(vchPubKey));

    // Fill listctrl with wallet transactions
    RefreshListCtrl();
}

CMainFrame::~CMainFrame()
{
    pframeMain = NULL;
    delete ptaskbaricon;
    ptaskbaricon = NULL;
}

void CMainFrame::OnKeyDown(wxKeyEvent& event) 
{ 
    HandleCtrlA(event); 
}

void CMainFrame::OnNotebookPageChanged(wxNotebookEvent& event)
{
    event.Skip();
    nPage = event.GetSelection();
    if (nPage == ALL)
    {
        m_listCtrl = m_listCtrlAll;
        fShowGenerated = true;
        fShowSent = true;
        fShowReceived = true;
    }
    else if (nPage == SENTRECEIVED)
    {
        m_listCtrl = m_listCtrlSentReceived;
        fShowGenerated = false;
        fShowSent = true;
        fShowReceived = true;
    }
    else if (nPage == SENT)
    {
        m_listCtrl = m_listCtrlSent;
        fShowGenerated = false;
        fShowSent = true;
        fShowReceived = false;
    }
    else if (nPage == RECEIVED)
    {
        m_listCtrl = m_listCtrlReceived;
        fShowGenerated = false;
        fShowSent = false;
        fShowReceived = true;
    }
    RefreshListCtrl();
    m_listCtrl->SetFocus();
}

void CMainFrame::OnClose(wxCloseEvent& event)
{
    if (fMinimizeOnClose && event.CanVeto() && !IsIconized())
    {
        // Divert close to minimize
        event.Veto();
        fClosedToTray = true;
        Iconize(true);
    }
    else
    {
        Destroy();
        CreateThread(Shutdown, NULL);
    }
}

void CMainFrame::OnIconize(wxIconizeEvent& event)
{
    event.Skip();
    // Hide the task bar button when minimized.
    // Event is sent when the frame is minimized or restored.
    // wxWidgets 2.8.9 doesn't have IsIconized() so there's no way
    // to get rid of the deprecated warning.  Just ignore it.
    if (!event.Iconized())
        fClosedToTray = false;
#if defined(__WXGTK__) || defined(__WXMAC_OSX__)
    if (GetBoolArg("-minimizetotray")) {
#endif
    // The tray icon sometimes disappears on ubuntu karmic
    // Hiding the taskbar button doesn't work cleanly on ubuntu lucid
    // Reports of CPU peg on 64-bit linux
    if (fMinimizeToTray && event.Iconized())
        fClosedToTray = true;
    Show(!fClosedToTray);
    ptaskbaricon->Show(fMinimizeToTray || fClosedToTray);
#if defined(__WXGTK__) || defined(__WXMAC_OSX__)
    }
#endif
}

void CMainFrame::OnMouseEvents(wxMouseEvent& event)
{
    event.Skip();
    RandAddSeed();
    RAND_add(&event.m_x, sizeof(event.m_x), 0.25);
    RAND_add(&event.m_y, sizeof(event.m_y), 0.25);
}

void CMainFrame::OnListColBeginDrag(wxListEvent& event)
{
    // Hidden columns not resizeable
    if (event.GetColumn() <= 1 && !fDebug)
        event.Veto();
    else
        event.Skip();
}

int CMainFrame::GetSortIndex(const string& strSort)
{
#ifdef __WXMSW__
    return 0;
#else
    // The wx generic listctrl implementation used on GTK doesn't sort,
    // so we have to do it ourselves.  Remember, we sort in reverse order.
    // In the wx generic implementation, they store the list of items
    // in a vector, so indexed lookups are fast, but inserts are slower
    // the closer they are to the top.
    int low = 0;
    int high = m_listCtrl->GetItemCount();
    while (low < high)
    {
        int mid = low + ((high - low) / 2);
        if (strSort.compare(m_listCtrl->GetItemText(mid).c_str()) >= 0)
            high = mid;
        else
            low = mid + 1;
    }
    return low;
#endif
}

void CMainFrame::InsertLine(bool fNew, int nIndex, uint256 hashKey, string strSort, const wxColour& colour, const wxString& str2, const wxString& str3, const wxString& str4, const wxString& str5, const wxString& str6)
{
    strSort = " " + strSort;       // leading space to workaround wx2.9.0 ubuntu 9.10 bug
    long nData = *(long*)&hashKey; //  where first char of hidden column is displayed

    // Find item
    if (!fNew && nIndex == -1)
    {
        string strHash = " " + hashKey.ToString();
        while ((nIndex = m_listCtrl->FindItem(nIndex, nData)) != -1)
            if (GetItemText(m_listCtrl, nIndex, 1) == strHash)
                break;
    }

    // fNew is for blind insert, only use if you're sure it's new
    if (fNew || nIndex == -1)
    {
        nIndex = m_listCtrl->InsertItem(GetSortIndex(strSort), strSort);
    }
    else
    {
        // If sort key changed, must delete and reinsert to make it relocate
        if (GetItemText(m_listCtrl, nIndex, 0) != strSort)
        {
            m_listCtrl->DeleteItem(nIndex);
            nIndex = m_listCtrl->InsertItem(GetSortIndex(strSort), strSort);
        }
    }

    m_listCtrl->SetItem(nIndex, 1, " " + hashKey.ToString());
    m_listCtrl->SetItem(nIndex, 2, str2);
    m_listCtrl->SetItem(nIndex, 3, str3);
    m_listCtrl->SetItem(nIndex, 4, str4);
    m_listCtrl->SetItem(nIndex, 5, str5);
    m_listCtrl->SetItem(nIndex, 6, str6);
    m_listCtrl->SetItemData(nIndex, nData);
    SetItemTextColour(m_listCtrl, nIndex, colour);
}

bool CMainFrame::DeleteLine(uint256 hashKey)
{
    long nData = *(long*)&hashKey;

    // Find item
    int nIndex = -1;
    string strHash = " " + hashKey.ToString();
    while ((nIndex = m_listCtrl->FindItem(nIndex, nData)) != -1)
        if (GetItemText(m_listCtrl, nIndex, 1) == strHash)
            break;

    if (nIndex != -1)
        m_listCtrl->DeleteItem(nIndex);

    return nIndex != -1;
}

bool CMainFrame::InsertTransaction(const CWalletTx& wtx, bool fNew, int nIndex)
{
    int64 nTime = wtx.nTimeDisplayed = wtx.GetTxTime();
    int64 nCredit = wtx.GetCredit(true);
    int64 nDebit = wtx.GetDebit();
    int64 nNet = nCredit - nDebit;
    uint256 hash = wtx.GetHash();
    string strStatus = FormatTxStatus(wtx);
    bool fConfirmed = wtx.fConfirmedDisplayed = wtx.IsConfirmed();
    wxColour colour = (fConfirmed ? wxColour(0,0,0) : wxColour(128,128,128));
    map<string, string> mapValue = wtx.mapValue;
    wtx.nLinesDisplayed = 1;
    nListViewUpdated++;

    // Filter
    if (wtx.IsCoinBase())
    {
        // Don't show generated coin until confirmed by at least one block after it
        // so we don't get the user's hopes up until it looks like it's probably accepted.
        //
        // It is not an error when generated blocks are not accepted.  By design,
        // some percentage of blocks, like 10% or more, will end up not accepted.
        // This is the normal mechanism by which the network copes with latency.
        //
        // We display regular transactions right away before any confirmation
        // because they can always get into some block eventually.  Generated coins
        // are special because if their block is not accepted, they are not valid.
        //
        if (wtx.GetDepthInMainChain() < 2)
        {
            wtx.nLinesDisplayed = 0;
            return false;
        }

        if (!fShowGenerated)
            return false;
    }

    // Find the block the tx is in
    CBlockIndex* pindex = NULL;
    map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(wtx.hashBlock);
    if (mi != mapBlockIndex.end())
        pindex = (*mi).second;

    // Sort order, unrecorded transactions sort to the top
    string strSort = strprintf("%010d-%01d-%010u",
        (pindex ? pindex->nHeight : INT_MAX),
        (wtx.IsCoinBase() ? 1 : 0),
        wtx.nTimeReceived);

    // Insert line
    if (nNet > 0 || wtx.IsCoinBase())
    {
        //
        // Credit
        //
        string strDescription;
        if (wtx.IsCoinBase())
        {
            // Generated
            strDescription = _("Generated");
            if (nCredit == 0)
            {
                int64 nUnmatured = 0;
                foreach(const CTxOut& txout, wtx.vout)
                    nUnmatured += txout.GetCredit();
                if (wtx.IsInMainChain())
                {
                    strDescription = strprintf(_("Generated (%s matures in %d more blocks)"), FormatMoney(nUnmatured).c_str(), wtx.GetBlocksToMaturity());

                    // Check if the block was requested by anyone
                    if (GetAdjustedTime() - wtx.nTimeReceived > 2 * 60 && wtx.GetRequestCount() == 0)
                        strDescription = _("Generated - Warning: This block was not received by any other nodes and will probably not be accepted!");
                }
                else
                {
                    strDescription = _("Generated (not accepted)");
                }
            }
        }
        else if (!mapValue["from"].empty() || !mapValue["message"].empty())
        {
            // Received by IP connection
            if (!fShowReceived)
                return false;
            if (!mapValue["from"].empty())
                strDescription += _("From: ") + mapValue["from"];
            if (!mapValue["message"].empty())
            {
                if (!strDescription.empty())
                    strDescription += " - ";
                strDescription += mapValue["message"];
            }
        }
        else
        {
            // Received by Bitcoin Address
            if (!fShowReceived)
                return false;
            foreach(const CTxOut& txout, wtx.vout)
            {
                if (txout.IsMine())
                {
                    vector<unsigned char> vchPubKey;
                    if (ExtractPubKey(txout.scriptPubKey, true, vchPubKey))
                    {
                        CRITICAL_BLOCK(cs_mapAddressBook)
                        {
                            //strDescription += _("Received payment to ");
                            //strDescription += _("Received with address ");
                            strDescription += _("Received with: ");
                            string strAddress = PubKeyToAddress(vchPubKey);
                            map<string, string>::iterator mi = mapAddressBook.find(strAddress);
                            if (mi != mapAddressBook.end() && !(*mi).second.empty())
                            {
                                string strLabel = (*mi).second;
                                strDescription += strAddress.substr(0,12) + "... ";
                                strDescription += "(" + strLabel + ")";
                            }
                            else
                                strDescription += strAddress;
                        }
                    }
                    break;
                }
            }
        }

        string strCredit = FormatMoney(nNet, true);
        if (!fConfirmed)
            strCredit = "[" + strCredit + "]";

        InsertLine(fNew, nIndex, hash, strSort, colour,
                   strStatus,
                   nTime ? DateTimeStr(nTime) : "",
                   SingleLine(strDescription),
                   "",
                   strCredit);
    }
    else
    {
        bool fAllFromMe = true;
        foreach(const CTxIn& txin, wtx.vin)
            fAllFromMe = fAllFromMe && txin.IsMine();

        bool fAllToMe = true;
        foreach(const CTxOut& txout, wtx.vout)
            fAllToMe = fAllToMe && txout.IsMine();

        if (fAllFromMe && fAllToMe)
        {
            // Payment to self
            int64 nChange = wtx.GetChange();
            InsertLine(fNew, nIndex, hash, strSort, colour,
                       strStatus,
                       nTime ? DateTimeStr(nTime) : "",
                       _("Payment to yourself"),
                       FormatMoney(-(nDebit - nChange), true),
                       FormatMoney(nCredit - nChange, true));
        }
        else if (fAllFromMe)
        {
            //
            // Debit
            //
            if (!fShowSent)
                return false;

            int64 nTxFee = nDebit - wtx.GetValueOut();
            wtx.nLinesDisplayed = 0;
            for (int nOut = 0; nOut < wtx.vout.size(); nOut++)
            {
                const CTxOut& txout = wtx.vout[nOut];
                if (txout.IsMine())
                    continue;

                string strAddress;
                if (!mapValue["to"].empty())
                {
                    // Sent to IP
                    strAddress = mapValue["to"];
                }
                else
                {
                    // Sent to Bitcoin Address
                    uint160 hash160;
                    if (ExtractHash160(txout.scriptPubKey, hash160))
                        strAddress = Hash160ToAddress(hash160);
                }

                string strDescription = _("To: ");
                CRITICAL_BLOCK(cs_mapAddressBook)
                    if (mapAddressBook.count(strAddress) && !mapAddressBook[strAddress].empty())
                        strDescription += mapAddressBook[strAddress] + " ";
                strDescription += strAddress;
                if (!mapValue["message"].empty())
                {
                    if (!strDescription.empty())
                        strDescription += " - ";
                    strDescription += mapValue["message"];
                }
                else if (!mapValue["comment"].empty())
                {
                    if (!strDescription.empty())
                        strDescription += " - ";
                    strDescription += mapValue["comment"];
                }

                int64 nValue = txout.nValue;
                if (nTxFee > 0)
                {
                    nValue += nTxFee;
                    nTxFee = 0;
                }

                InsertLine(fNew, nIndex, hash, strprintf("%s-%d", strSort.c_str(), nOut), colour,
                           strStatus,
                           nTime ? DateTimeStr(nTime) : "",
                           SingleLine(strDescription),
                           FormatMoney(-nValue, true),
                           "");
                nIndex = -1;
                wtx.nLinesDisplayed++;
            }
        }
        else
        {
            //
            // Mixed debit transaction, can't break down payees
            //
            bool fAllMine = true;
            foreach(const CTxOut& txout, wtx.vout)
                fAllMine = fAllMine && txout.IsMine();
            foreach(const CTxIn& txin, wtx.vin)
                fAllMine = fAllMine && txin.IsMine();

            InsertLine(fNew, nIndex, hash, strSort, colour,
                       strStatus,
                       nTime ? DateTimeStr(nTime) : "",
                       "",
                       FormatMoney(nNet, true),
                       "");
        }
    }

    return true;
}

void CMainFrame::RefreshListCtrl()
{
    fRefreshListCtrl = true;
    ::wxWakeUpIdle();
}

void CMainFrame::OnIdle(wxIdleEvent& event)
{
    if (fRefreshListCtrl)
    {
        // Collect list of wallet transactions and sort newest first
        bool fEntered = false;
        vector<pair<unsigned int, uint256> > vSorted;
        TRY_CRITICAL_BLOCK(cs_mapWallet)
        {
            printf("RefreshListCtrl starting\n");
            fEntered = true;
            fRefreshListCtrl = false;
            vWalletUpdated.clear();

            // Do the newest transactions first
            vSorted.reserve(mapWallet.size());
            for (map<uint256, CWalletTx>::iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
            {
                const CWalletTx& wtx = (*it).second;
                unsigned int nTime = UINT_MAX - wtx.GetTxTime();
                vSorted.push_back(make_pair(nTime, (*it).first));
            }
            m_listCtrl->DeleteAllItems();
        }
        if (!fEntered)
            return;

        sort(vSorted.begin(), vSorted.end());

        // Fill list control
        for (int i = 0; i < vSorted.size();)
        {
            if (fShutdown)
                return;
            bool fEntered = false;
            TRY_CRITICAL_BLOCK(cs_mapWallet)
            {
                fEntered = true;
                uint256& hash = vSorted[i++].second;
                map<uint256, CWalletTx>::iterator mi = mapWallet.find(hash);
                if (mi != mapWallet.end())
                    InsertTransaction((*mi).second, true);
            }
            if (!fEntered || i == 100 || i % 500 == 0)
                wxYield();
        }

        printf("RefreshListCtrl done\n");

        // Update transaction total display
        MainFrameRepaint();
    }
    else
    {
        // Check for time updates
        static int64 nLastTime;
        if (GetTime() > nLastTime + 30)
        {
            TRY_CRITICAL_BLOCK(cs_mapWallet)
            {
                nLastTime = GetTime();
                for (map<uint256, CWalletTx>::iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
                {
                    CWalletTx& wtx = (*it).second;
                    if (wtx.nTimeDisplayed && wtx.nTimeDisplayed != wtx.GetTxTime())
                        InsertTransaction(wtx, false);
                }
            }
        }
    }
}

void CMainFrame::RefreshStatusColumn()
{
    static int nLastTop;
    static CBlockIndex* pindexLastBest;
    static unsigned int nLastRefreshed;

    int nTop = max((int)m_listCtrl->GetTopItem(), 0);
    if (nTop == nLastTop && pindexLastBest == pindexBest)
        return;

    TRY_CRITICAL_BLOCK(cs_mapWallet)
    {
        int nStart = nTop;
        int nEnd = min(nStart + 100, m_listCtrl->GetItemCount());

        if (pindexLastBest == pindexBest && nLastRefreshed == nListViewUpdated)
        {
            // If no updates, only need to do the part that moved onto the screen
            if (nStart >= nLastTop && nStart < nLastTop + 100)
                nStart = nLastTop + 100;
            if (nEnd >= nLastTop && nEnd < nLastTop + 100)
                nEnd = nLastTop;
        }
        nLastTop = nTop;
        pindexLastBest = pindexBest;
        nLastRefreshed = nListViewUpdated;

        for (int nIndex = nStart; nIndex < min(nEnd, m_listCtrl->GetItemCount()); nIndex++)
        {
            uint256 hash((string)GetItemText(m_listCtrl, nIndex, 1));
            map<uint256, CWalletTx>::iterator mi = mapWallet.find(hash);
            if (mi == mapWallet.end())
            {
                printf("CMainFrame::RefreshStatusColumn() : tx not found in mapWallet\n");
                continue;
            }
            CWalletTx& wtx = (*mi).second;
            if (wtx.IsCoinBase() ||
                wtx.GetTxTime() != wtx.nTimeDisplayed ||
                wtx.IsConfirmed() != wtx.fConfirmedDisplayed)
            {
                if (!InsertTransaction(wtx, false, nIndex))
                    m_listCtrl->DeleteItem(nIndex--);
            }
            else
            {
                m_listCtrl->SetItem(nIndex, 2, FormatTxStatus(wtx));
            }
        }
    }
}

void CMainFrame::OnPaint(wxPaintEvent& event)
{
    event.Skip();
    if (fRefresh)
    {
        fRefresh = false;
        Refresh();
    }
}

void CMainFrame::OnPaintListCtrl(wxPaintEvent& event)
{
    // Skip lets the listctrl do the paint, we're just hooking the message
    event.Skip();

    if (ptaskbaricon)
        ptaskbaricon->UpdateTooltip();

    //
    // Slower stuff
    //
    static int nTransactionCount;
    bool fPaintedBalance = false;
    if (GetTimeMillis() - nLastRepaintTime >= nRepaintInterval)
    {
        nLastRepaint = nNeedRepaint;
        nLastRepaintTime = GetTimeMillis();

        // Update listctrl contents
        if (!vWalletUpdated.empty())
        {
            TRY_CRITICAL_BLOCK(cs_mapWallet)
            {
                string strTop;
                if (m_listCtrl->GetItemCount())
                    strTop = (string)m_listCtrl->GetItemText(0);
                foreach(uint256 hash, vWalletUpdated)
                {
                    map<uint256, CWalletTx>::iterator mi = mapWallet.find(hash);
                    if (mi != mapWallet.end())
                        InsertTransaction((*mi).second, false);
                }
                vWalletUpdated.clear();
                if (m_listCtrl->GetItemCount() && strTop != (string)m_listCtrl->GetItemText(0))
                    m_listCtrl->ScrollList(0, INT_MIN/2);
            }
        }

        // Balance total
        TRY_CRITICAL_BLOCK(cs_mapWallet)
        {
            fPaintedBalance = true;
            m_staticTextBalance->SetLabel(FormatMoney(GetBalance()) + "  ");

            // Count hidden and multi-line transactions
            nTransactionCount = 0;
            for (map<uint256, CWalletTx>::iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
            {
                CWalletTx& wtx = (*it).second;
                nTransactionCount += wtx.nLinesDisplayed;
            }
        }
    }
    if (!vWalletUpdated.empty() || !fPaintedBalance)
        nNeedRepaint++;

    // Update status column of visible items only
    RefreshStatusColumn();

    // Update status bar
    static string strPrevWarning;
    string strWarning = GetWarnings("statusbar");
    if (strWarning != "")
        m_statusBar->SetStatusText(string("    ") + _(strWarning), 0);
    else if (strPrevWarning != "")
        m_statusBar->SetStatusText("", 0);
    strPrevWarning = strWarning;

    string strGen = "";
    if (fGenerateBitcoins)
        strGen = _("    Generating");
    if (fGenerateBitcoins && vNodes.empty())
        strGen = _("(not connected)");
    m_statusBar->SetStatusText(strGen, 1);

    string strStatus = strprintf(_("     %d connections     %d blocks     %d transactions"), vNodes.size(), nBestHeight, nTransactionCount);
    m_statusBar->SetStatusText(strStatus, 2);

    // Update receiving address
    string strDefaultAddress = PubKeyToAddress(vchDefaultKey);
    if (m_textCtrlAddress->GetValue() != strDefaultAddress)
        m_textCtrlAddress->SetValue(strDefaultAddress);
}

void CMainFrame::OnUIThreadCall(wxCommandEvent& event)
{
    boost::function0<void>* pfn = (boost::function0<void>*)event.GetClientData();
    (*pfn)();
    delete pfn;
}

void CMainFrame::OnMenuFileExit(wxCommandEvent& event)
{
    // File->Exit
    Close(true);
}

void CMainFrame::OnMenuOptionsGenerate(wxCommandEvent& event)
{
    // Options->Generate Coins
    GenerateBitcoins(event.IsChecked());
}

void CMainFrame::OnUpdateUIOptionsGenerate(wxUpdateUIEvent& event)
{
    event.Check(fGenerateBitcoins);
}

void CMainFrame::OnMenuOptionsChangeYourAddress(wxCommandEvent& event)
{
    // Options->Your Receiving Addresses
    CAddressBookDialog dialog(this, "", CAddressBookDialog::RECEIVING, false);
    if (!dialog.ShowModal())
        return;
}

void CMainFrame::OnMenuOptionsOptions(wxCommandEvent& event)
{
    // Options->Options
    COptionsDialog dialog(this);
    dialog.ShowModal();
}

void CMainFrame::OnMenuHelpAbout(wxCommandEvent& event)
{
    // Help->About
    CAboutDialog dialog(this);
    dialog.ShowModal();
}

void CMainFrame::OnButtonSend(wxCommandEvent& event)
{
    // Toolbar: Send
    CSendDialog dialog(this);
    dialog.ShowModal();
}

void CMainFrame::OnButtonAddressBook(wxCommandEvent& event)
{
    // Toolbar: Address Book
    CAddressBookDialog dialogAddr(this, "", CAddressBookDialog::SENDING, false);
    if (dialogAddr.ShowModal() == 2)
    {
        // Send
        CSendDialog dialogSend(this, dialogAddr.GetSelectedAddress());
        dialogSend.ShowModal();
    }
}

void CMainFrame::OnSetFocusAddress(wxFocusEvent& event)
{
    // Automatically select-all when entering window
    event.Skip();
    m_textCtrlAddress->SetSelection(-1, -1);
    fOnSetFocusAddress = true;
}

void CMainFrame::OnMouseEventsAddress(wxMouseEvent& event)
{
    event.Skip();
    if (fOnSetFocusAddress)
        m_textCtrlAddress->SetSelection(-1, -1);
    fOnSetFocusAddress = false;
}

void CMainFrame::OnButtonNew(wxCommandEvent& event)
{
    // Ask name
    CGetTextFromUserDialog dialog(this,
        _("New Receiving Address"),
        _("You should use a new address for each payment you receive.\n\nLabel"),
        "");
    if (!dialog.ShowModal())
        return;
    string strName = dialog.GetValue();

    // Generate new key
    string strAddress = PubKeyToAddress(GetKeyFromKeyPool());

    // Save
    SetAddressBookName(strAddress, strName);
    SetDefaultReceivingAddress(strAddress);
}

void CMainFrame::OnButtonCopy(wxCommandEvent& event)
{
    // Copy address box to clipboard
    if (wxTheClipboard->Open())
    {
        wxTheClipboard->SetData(new wxTextDataObject(m_textCtrlAddress->GetValue()));
        wxTheClipboard->Close();
    }
}

void CMainFrame::OnListItemActivated(wxListEvent& event)
{
    uint256 hash((string)GetItemText(m_listCtrl, event.GetIndex(), 1));
    CWalletTx wtx;
    CRITICAL_BLOCK(cs_mapWallet)
    {
        map<uint256, CWalletTx>::iterator mi = mapWallet.find(hash);
        if (mi == mapWallet.end())
        {
            printf("CMainFrame::OnListItemActivated() : tx not found in mapWallet\n");
            return;
        }
        wtx = (*mi).second;
    }
    CTxDetailsDialog dialog(this, wtx);
    dialog.ShowModal();
    //CTxDetailsDialog* pdialog = new CTxDetailsDialog(this, wtx);
    //pdialog->Show();
}
