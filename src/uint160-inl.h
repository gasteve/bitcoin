inline bool operator==(const uint160& a, uint64 b)                           { return (base_uint160)a == b; }
inline bool operator!=(const uint160& a, uint64 b)                           { return (base_uint160)a != b; }
inline const uint160 operator<<(const base_uint160& a, unsigned int shift)   { return uint160(a) <<= shift; }
inline const uint160 operator>>(const base_uint160& a, unsigned int shift)   { return uint160(a) >>= shift; }
inline const uint160 operator<<(const uint160& a, unsigned int shift)        { return uint160(a) <<= shift; }
inline const uint160 operator>>(const uint160& a, unsigned int shift)        { return uint160(a) >>= shift; }

inline const uint160 operator^(const base_uint160& a, const base_uint160& b) { return uint160(a) ^= b; }
inline const uint160 operator&(const base_uint160& a, const base_uint160& b) { return uint160(a) &= b; }
inline const uint160 operator|(const base_uint160& a, const base_uint160& b) { return uint160(a) |= b; }
inline const uint160 operator+(const base_uint160& a, const base_uint160& b) { return uint160(a) += b; }
inline const uint160 operator-(const base_uint160& a, const base_uint160& b) { return uint160(a) -= b; }

inline bool operator<(const base_uint160& a, const uint160& b)          { return (base_uint160)a <  (base_uint160)b; }
inline bool operator<=(const base_uint160& a, const uint160& b)         { return (base_uint160)a <= (base_uint160)b; }
inline bool operator>(const base_uint160& a, const uint160& b)          { return (base_uint160)a >  (base_uint160)b; }
inline bool operator>=(const base_uint160& a, const uint160& b)         { return (base_uint160)a >= (base_uint160)b; }
inline bool operator==(const base_uint160& a, const uint160& b)         { return (base_uint160)a == (base_uint160)b; }
inline bool operator!=(const base_uint160& a, const uint160& b)         { return (base_uint160)a != (base_uint160)b; }
inline const uint160 operator^(const base_uint160& a, const uint160& b) { return (base_uint160)a ^  (base_uint160)b; }
inline const uint160 operator&(const base_uint160& a, const uint160& b) { return (base_uint160)a &  (base_uint160)b; }
inline const uint160 operator|(const base_uint160& a, const uint160& b) { return (base_uint160)a |  (base_uint160)b; }
inline const uint160 operator+(const base_uint160& a, const uint160& b) { return (base_uint160)a +  (base_uint160)b; }
inline const uint160 operator-(const base_uint160& a, const uint160& b) { return (base_uint160)a -  (base_uint160)b; }

inline bool operator<(const uint160& a, const base_uint160& b)          { return (base_uint160)a <  (base_uint160)b; }
inline bool operator<=(const uint160& a, const base_uint160& b)         { return (base_uint160)a <= (base_uint160)b; }
inline bool operator>(const uint160& a, const base_uint160& b)          { return (base_uint160)a >  (base_uint160)b; }
inline bool operator>=(const uint160& a, const base_uint160& b)         { return (base_uint160)a >= (base_uint160)b; }
inline bool operator==(const uint160& a, const base_uint160& b)         { return (base_uint160)a == (base_uint160)b; }
inline bool operator!=(const uint160& a, const base_uint160& b)         { return (base_uint160)a != (base_uint160)b; }
inline const uint160 operator^(const uint160& a, const base_uint160& b) { return (base_uint160)a ^  (base_uint160)b; }
inline const uint160 operator&(const uint160& a, const base_uint160& b) { return (base_uint160)a &  (base_uint160)b; }
inline const uint160 operator|(const uint160& a, const base_uint160& b) { return (base_uint160)a |  (base_uint160)b; }
inline const uint160 operator+(const uint160& a, const base_uint160& b) { return (base_uint160)a +  (base_uint160)b; }
inline const uint160 operator-(const uint160& a, const base_uint160& b) { return (base_uint160)a -  (base_uint160)b; }

inline bool operator<(const uint160& a, const uint160& b)               { return (base_uint160)a <  (base_uint160)b; }
inline bool operator<=(const uint160& a, const uint160& b)              { return (base_uint160)a <= (base_uint160)b; }
inline bool operator>(const uint160& a, const uint160& b)               { return (base_uint160)a >  (base_uint160)b; }
inline bool operator>=(const uint160& a, const uint160& b)              { return (base_uint160)a >= (base_uint160)b; }
inline bool operator==(const uint160& a, const uint160& b)              { return (base_uint160)a == (base_uint160)b; }
inline bool operator!=(const uint160& a, const uint160& b)              { return (base_uint160)a != (base_uint160)b; }
inline const uint160 operator^(const uint160& a, const uint160& b)      { return (base_uint160)a ^  (base_uint160)b; }
inline const uint160 operator&(const uint160& a, const uint160& b)      { return (base_uint160)a &  (base_uint160)b; }
inline const uint160 operator|(const uint160& a, const uint160& b)      { return (base_uint160)a |  (base_uint160)b; }
inline const uint160 operator+(const uint160& a, const uint160& b)      { return (base_uint160)a +  (base_uint160)b; }
inline const uint160 operator-(const uint160& a, const uint160& b)      { return (base_uint160)a -  (base_uint160)b; }
