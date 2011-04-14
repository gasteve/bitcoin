#include "CAlert.h"
#include "main.h"
#include "util.h"
#include "CNode.h"
#include "CKey.h"
#include "CCriticalBlock.h"

CAlert::CAlert()
{
	SetNull();
}

void CAlert::SetNull()
{
	CUnsignedAlert::SetNull();
	vchMsg.clear();
	vchSig.clear();
}

bool CAlert::IsNull() const
{
	return (nExpiration == 0);
}

uint256 CAlert::GetHash() const
{
	return SerializeHash(*this);
}

bool CAlert::IsInEffect() const
{
	return (GetAdjustedTime() < nExpiration);
}

bool CAlert::Cancels(const CAlert& alert) const
{
	if (!IsInEffect())
		return false; // this was a no-op before 31403
	return (alert.nID <= nCancel || setCancel.count(alert.nID));
}

bool CAlert::AppliesTo(int nVersion, string strSubVerIn) const
{
	return (IsInEffect() &&
			nMinVer <= nVersion && nVersion <= nMaxVer &&
			(setSubVer.empty() || setSubVer.count(strSubVerIn)));
}

bool CAlert::AppliesToMe() const
{
	return AppliesTo(VERSION, ::pszSubVer);
}

bool CAlert::RelayTo(CNode* pnode) const
{
	if (!IsInEffect())
		return false;
	// returns true if wasn't already contained in the set
	if (pnode->setKnown.insert(GetHash()).second)
	{
		if (AppliesTo(pnode->nVersion, pnode->strSubVer) ||
			AppliesToMe() ||
			GetAdjustedTime() < nRelayUntil)
		{
			pnode->PushMessage("alert", *this);
			return true;
		}
	}
	return false;
}

bool CAlert::CheckSignature()
{
	CKey key;
	if (!key.SetPubKey(ParseHex("04fc9702847840aaf195de8442ebecedf5b095cdbb9bc716bda9110971b28a49e0ead8564ff0db22209e0374782c093bb899692d524e9d6a6956e7c5ecbcd68284")))
		return error("CAlert::CheckSignature() : SetPubKey failed");
	if (!key.Verify(Hash(vchMsg.begin(), vchMsg.end()), vchSig))
		return error("CAlert::CheckSignature() : verify signature failed");

	// Now unserialize the data
	CDataStream sMsg(vchMsg);
	sMsg >> *(CUnsignedAlert*)this;
	return true;
}

bool CAlert::ProcessAlert()
{
    if (!CheckSignature())
        return false;
    if (!IsInEffect())
        return false;

    CRITICAL_BLOCK(cs_mapAlerts)
    {
        // Cancel previous alerts
        for (map<uint256, CAlert>::iterator mi = mapAlerts.begin(); mi != mapAlerts.end();)
        {
            const CAlert& alert = (*mi).second;
            if (Cancels(alert))
            {
                printf("cancelling alert %d\n", alert.nID);
                mapAlerts.erase(mi++);
            }
            else if (!alert.IsInEffect())
            {
                printf("expiring alert %d\n", alert.nID);
                mapAlerts.erase(mi++);
            }
            else
                mi++;
        }

        // Check if this alert has been cancelled
        foreach(PAIRTYPE(const uint256, CAlert)& item, mapAlerts)
        {
            const CAlert& alert = item.second;
            if (alert.Cancels(*this))
            {
                printf("alert already cancelled by %d\n", alert.nID);
                return false;
            }
        }

        // Add to mapAlerts
        mapAlerts.insert(make_pair(GetHash(), *this));
    }

    printf("accepted alert %d, AppliesToMe()=%d\n", nID, AppliesToMe());
    MainFrameRepaint();
    return true;
}

