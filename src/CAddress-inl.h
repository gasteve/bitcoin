inline bool operator==(const CAddress& a, const CAddress& b)
{
        return (memcmp(a.pchReserved, b.pchReserved, sizeof(a.pchReserved)) == 0 &&
                        a.ip   == b.ip &&
                        a.port == b.port);
}

inline bool operator!=(const CAddress& a, const CAddress& b)
{
        return (!(a == b));
}

inline bool operator<(const CAddress& a, const CAddress& b)
{
        int ret = memcmp(a.pchReserved, b.pchReserved, sizeof(a.pchReserved));
        if (ret < 0)
                return true;
        else if (ret == 0)
        {
                if (ntohl(a.ip) < ntohl(b.ip))
                        return true;
                else if (a.ip == b.ip)
                        return ntohs(a.port) < ntohs(b.port);
        }
        return false;
}

