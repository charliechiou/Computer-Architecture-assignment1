static inline int64_t getbit(int64_t value, int n)
{
    return (value >> n) & 1;
}