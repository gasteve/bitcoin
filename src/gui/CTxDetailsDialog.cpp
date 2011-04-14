//////////////////////////////////////////////////////////////////////////////
//
// CTxDetailsDialog
//

#include "CTxDetailsDialog.h"

#include "main.h"
#include "script.h"
#include "base58.h"
#include "CCriticalBlock.h"

CTxDetailsDialog::CTxDetailsDialog(wxWindow* parent, CWalletTx wtx) : CTxDetailsDialogBase(parent)
{
    CRITICAL_BLOCK(cs_mapAddressBook)
    {
        string strHTML;
        strHTML.reserve(4000);
        strHTML += "<html><font face='verdana, arial, helvetica, sans-serif'>";

        int64 nTime = wtx.GetTxTime();
        int64 nCredit = wtx.GetCredit();
        int64 nDebit = wtx.GetDebit();
        int64 nNet = nCredit - nDebit;



        strHTML += _("<b>Status:</b> ") + FormatTxStatus(wtx);
        int nRequests = wtx.GetRequestCount();
        if (nRequests != -1)
        {
            if (nRequests == 0)
                strHTML += _(", has not been successfully broadcast yet");
            else if (nRequests == 1)
                strHTML += strprintf(_(", broadcast through %d node"), nRequests);
            else
                strHTML += strprintf(_(", broadcast through %d nodes"), nRequests);
        }
        strHTML += "<br>";

        strHTML += _("<b>Date:</b> ") + (nTime ? DateTimeStr(nTime) : "") + "<br>";


        //
        // From
        //
        if (wtx.IsCoinBase())
        {
            strHTML += _("<b>Source:</b> Generated<br>");
        }
        else if (!wtx.mapValue["from"].empty())
        {
            // Online transaction
            if (!wtx.mapValue["from"].empty())
                strHTML += _("<b>From:</b> ") + HtmlEscape(wtx.mapValue["from"]) + "<br>";
        }
        else
        {
            // Offline transaction
            if (nNet > 0)
            {
                // Credit
                foreach(const CTxOut& txout, wtx.vout)
                {
                    if (txout.IsMine())
                    {
                        vector<unsigned char> vchPubKey;
                        if (ExtractPubKey(txout.scriptPubKey, true, vchPubKey))
                        {
                            string strAddress = PubKeyToAddress(vchPubKey);
                            if (mapAddressBook.count(strAddress))
                            {
                                strHTML += string() + _("<b>From:</b> ") + _("unknown") + "<br>";
                                strHTML += _("<b>To:</b> ");
                                strHTML += HtmlEscape(strAddress);
                                if (!mapAddressBook[strAddress].empty())
                                    strHTML += _(" (yours, label: ") + mapAddressBook[strAddress] + ")";
                                else
                                    strHTML += _(" (yours)");
                                strHTML += "<br>";
                            }
                        }
                        break;
                    }
                }
            }
        }


        //
        // To
        //
        string strAddress;
        if (!wtx.mapValue["to"].empty())
        {
            // Online transaction
            strAddress = wtx.mapValue["to"];
            strHTML += _("<b>To:</b> ");
            if (mapAddressBook.count(strAddress) && !mapAddressBook[strAddress].empty())
                strHTML += mapAddressBook[strAddress] + " ";
            strHTML += HtmlEscape(strAddress) + "<br>";
        }


        //
        // Amount
        //
        if (wtx.IsCoinBase() && nCredit == 0)
        {
            //
            // Coinbase
            //
            int64 nUnmatured = 0;
            foreach(const CTxOut& txout, wtx.vout)
                nUnmatured += txout.GetCredit();
            strHTML += _("<b>Credit:</b> ");
            if (wtx.IsInMainChain())
                strHTML += strprintf(_("(%s matures in %d more blocks)"), FormatMoney(nUnmatured).c_str(), wtx.GetBlocksToMaturity());
            else
                strHTML += _("(not accepted)");
            strHTML += "<br>";
        }
        else if (nNet > 0)
        {
            //
            // Credit
            //
            strHTML += _("<b>Credit:</b> ") + FormatMoney(nNet) + "<br>";
        }
        else
        {
            bool fAllFromMe = true;
            foreach(const CTxIn& txin, wtx.vin)
                fAllFromMe = fAllFromMe && txin.IsMine();

            bool fAllToMe = true;
            foreach(const CTxOut& txout, wtx.vout)
                fAllToMe = fAllToMe && txout.IsMine();

            if (fAllFromMe)
            {
                //
                // Debit
                //
                foreach(const CTxOut& txout, wtx.vout)
                {
                    if (txout.IsMine())
                        continue;

                    if (wtx.mapValue["to"].empty())
                    {
                        // Offline transaction
                        uint160 hash160;
                        if (ExtractHash160(txout.scriptPubKey, hash160))
                        {
                            string strAddress = Hash160ToAddress(hash160);
                            strHTML += _("<b>To:</b> ");
                            if (mapAddressBook.count(strAddress) && !mapAddressBook[strAddress].empty())
                                strHTML += mapAddressBook[strAddress] + " ";
                            strHTML += strAddress;
                            strHTML += "<br>";
                        }
                    }

                    strHTML += _("<b>Debit:</b> ") + FormatMoney(-txout.nValue) + "<br>";
                }

                if (fAllToMe)
                {
                    // Payment to self
                    int64 nChange = wtx.GetChange();
                    int64 nValue = nCredit - nChange;
                    strHTML += _("<b>Debit:</b> ") + FormatMoney(-nValue) + "<br>";
                    strHTML += _("<b>Credit:</b> ") + FormatMoney(nValue) + "<br>";
                }

                int64 nTxFee = nDebit - wtx.GetValueOut();
                if (nTxFee > 0)
                    strHTML += _("<b>Transaction fee:</b> ") + FormatMoney(-nTxFee) + "<br>";
            }
            else
            {
                //
                // Mixed debit transaction
                //
                foreach(const CTxIn& txin, wtx.vin)
                    if (txin.IsMine())
                        strHTML += _("<b>Debit:</b> ") + FormatMoney(-txin.GetDebit()) + "<br>";
                foreach(const CTxOut& txout, wtx.vout)
                    if (txout.IsMine())
                        strHTML += _("<b>Credit:</b> ") + FormatMoney(txout.GetCredit()) + "<br>";
            }
        }

        strHTML += _("<b>Net amount:</b> ") + FormatMoney(nNet, true) + "<br>";


        //
        // Message
        //
        if (!wtx.mapValue["message"].empty())
            strHTML += string() + "<br><b>" + _("Message:") + "</b><br>" + HtmlEscape(wtx.mapValue["message"], true) + "<br>";
        if (!wtx.mapValue["comment"].empty())
            strHTML += string() + "<br><b>" + _("Comment:") + "</b><br>" + HtmlEscape(wtx.mapValue["comment"], true) + "<br>";

        if (wtx.IsCoinBase())
            strHTML += string() + "<br>" + _("Generated coins must wait 120 blocks before they can be spent.  When you generated this block, it was broadcast to the network to be added to the block chain.  If it fails to get into the chain, it will change to \"not accepted\" and not be spendable.  This may occasionally happen if another node generates a block within a few seconds of yours.") + "<br>";


        //
        // Debug view
        //
        if (fDebug)
        {
            strHTML += "<hr><br>debug print<br><br>";
            foreach(const CTxIn& txin, wtx.vin)
                if (txin.IsMine())
                    strHTML += "<b>Debit:</b> " + FormatMoney(-txin.GetDebit()) + "<br>";
            foreach(const CTxOut& txout, wtx.vout)
                if (txout.IsMine())
                    strHTML += "<b>Credit:</b> " + FormatMoney(txout.GetCredit()) + "<br>";

            strHTML += "<br><b>Transaction:</b><br>";
            strHTML += HtmlEscape(wtx.ToString(), true);

            strHTML += "<br><b>Inputs:</b><br>";
            CRITICAL_BLOCK(cs_mapWallet)
            {
                foreach(const CTxIn& txin, wtx.vin)
                {
                    COutPoint prevout = txin.prevout;
                    map<uint256, CWalletTx>::iterator mi = mapWallet.find(prevout.hash);
                    if (mi != mapWallet.end())
                    {
                        const CWalletTx& prev = (*mi).second;
                        if (prevout.n < prev.vout.size())
                        {
                            strHTML += HtmlEscape(prev.ToString(), true);
                            strHTML += " &nbsp;&nbsp; " + FormatTxStatus(prev) + ", ";
                            strHTML = strHTML + "IsMine=" + (prev.vout[prevout.n].IsMine() ? "true" : "false") + "<br>";
                        }
                    }
                }
            }
        }



        strHTML += "</font></html>";
        string(strHTML.begin(), strHTML.end()).swap(strHTML);
        m_htmlWin->SetPage(strHTML);
        m_buttonOK->SetFocus();
    }
}

void CTxDetailsDialog::OnButtonOK(wxCommandEvent& event)
{
    EndModal(false);
}
