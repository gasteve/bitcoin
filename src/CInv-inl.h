inline bool operator<(const CInv& a, const CInv& b)
{
        return (a.type < b.type || (a.type == b.type && a.hash < b.hash));
}

