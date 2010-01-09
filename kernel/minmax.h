
#ifndef MINMAX_H
#define MINMAX_H

static __inline__ long
minl(long lhs, long rhs)
{
        return lhs < rhs ? lhs : rhs;
}

static __inline__ unsigned long
minul(unsigned long lhs, unsigned long rhs)
{
        return lhs < rhs ? lhs : rhs;
}

static __inline__ long
maxl(long lhs, long rhs)
{
        return lhs > rhs ? lhs : rhs;
}

static __inline__ unsigned long
maxul(unsigned long lhs, unsigned long rhs)
{
        return lhs > rhs ? lhs : rhs;
}

#endif

