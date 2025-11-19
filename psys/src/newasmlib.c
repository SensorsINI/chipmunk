/* caged_process='cc -c $F$M'  caged_tabs=4 */


/* C version of the Caltech NEWASM library */
/* Dave Gillespie, 2-14-88 */


#define KEEP_P2C_DECLS

#if defined(rs6000)
# include <sys/time.h>
# include <time.h>
#elif defined(BSD)
# include <time.h>
# include <sys/time.h>
#else
# include <time.h>
#endif

#include <p2c/p2c.h>
#include <p2c/newasm.h>

#ifdef aux
#if (!defined(solaris) && !defined(__MACHTEN__))
# define LACKS_STRDUP    /* A/UX needs this. */
#endif
#endif

/* The following would benefit from assembly language if possible */

#define ROL_L(v,n)    (((v) << n) | ((unsigned long)(v) >> (32-(n))))
#define ROR_L(v,n)    (((unsigned long)(v) >> n) | ((v) << (32-(n))))

#define VROL_L(v,n)   ((v) = ROL_L(v,n))
#define VROR_L(v,n)   ((v) = ROR_L(v,n))

#if 1
# define ASR_L(v,n)   ((v) >> (n))       /* assuming shifts are signed */
#else
# define ASR_L(v,n)   (((v) >= 0) ? ((v) >> (n)) : ~((~(v)) >> (n)))
#endif
#define VASR_L(v,n)   ((v) = ASR_L(v,n))





/* Fill a block of memory with a repeating 4-byte pattern */

Void na_fill(d, w, n)
register long *d, w, n;
{
    if (n <= 0)
        return;
    if (w == 0 || w == -1) {
        memset(d, (int)w, (size_t)n);
        return;
    }
    if ((long)d & 1) {
        VROL_L(w, 8);
        *((uchar *)d) = w;
        d = (long *)(1 + (long)d);    /* yuck! */
        if (--n <= 0)
            return;
    }
    if (n == 1) {
        *((uchar *)d) = (w >> 24);
        return;
    }
    if ((long)d & 2) {
        VROL_L(w, 16);
        *((short *)d) = w;
        d = (long *)(2 + (long)d);
        n -= 2;
        if (n <= 0)
            return;
    }
    n -= 4;
    while (n >= 0) {
        *d++ = w;
        n -= 4;
    }
    if (n >= -2) {
        VROL_L(w, 16);
        *((short *)d) = w;
        d = (long *)(2 + (long)d);
        n -= 2;
    }
    if (n >= -3)
        *((uchar *)d) = (w >> 24);
}




/* Fill a block of memory with a word pattern */

Void na_fillword(d, w, n)
long *d;
long w, n;
{
    na_fill(d, ((unsigned short)w) | (w << 16), n * 2);
}




/* Exchange two blocks of memory */

Void na_exch(s, d, size)
long *s, *d;
register long size;
{
    switch (size) {

        case 0:
            return;

        case 1:
            {   register char t, *p1 = (char *)s, *p2 = (char *)d;
                t = *p1;
                *p1 = *p2;
                *p2 = t;
                return;
            }

        case 2:
            if (!(((long)s | (long)d) & 1)) {
                register short t, *p1 = (short *)s, *p2 = (short *)d;
                t = *p1;
                *p1 = *p2;
                *p2 = t;
                return;
            }
            break;

        case 4:
            if (!(((long)s | (long)d) & 3)) {
                register long *p1 = s, *p2 = d;
                size = *p1;
                *p1 = *p2;
                *p2 = size;
                return;
            }
            break;
    }
    {   register char t, *p1 = (char *)s, *p2 = (char *)d;
        while (--size >= 0) {
            t = *p1;
            *p1++ = *p2;
            *p2++ = t;
        }
    }
}




/* Scan for a byte equal to the target */

long na_scaneq(s, what, size)
long *s, what;
register long size;
{
    register uchar *cp = (uchar *)s;
    register uchar ch = what;

    while (--size >= 0) {
        if (*cp++ == ch) {
            return cp - (uchar *)s - 1;
        }
    }
    return cp - (uchar *)s;
}




/* Scan for a byte unequal to the target */

long na_scanne(s, what, size)
long *s, what;
register long size;
{
    register uchar *cp = (uchar *)s;
    register uchar ch = what;

    while (--size >= 0) {
        if (*cp++ != ch) {
            return cp - (uchar *)s - 1;
        }
    }
    return cp - (uchar *)s;
}




/* Stack-based allocation; approximate with malloc */

Void na_alloc(p, s)
Anyptr *p;
long s;
{
    *p = malloc(s);
}



Void na_outeralloc(p, s, parframe)
Anyptr *p;
long s, parframe;
{
    *p = malloc(s);
}



Void na_free(p)
Anyptr *p;
{
    free(*p);
    *p = NULL;
}




/* Bit operations */

boolean na_test(n, x)
register long n;
long x;
{
    if ((unsigned long)n > 31)
        return 0;
    return ((x & (1<<n)) != 0);
}


Void na_set(n, x)
register long n;
long *x;
{
    if ((unsigned long)n > 31)
        return;
    *x |= (1<<n);
}



Void na_clear(n, x)
register long n;
long *x;
{
    if ((unsigned long)n > 31)
        return;
    *x &= ~(1<<n);
}



long na_po2(n)
register long n;
{
    if ((unsigned long)n > 31)
        return 0;
    return 1<<n;
}



long na_log2(n)
long n;
{
    register unsigned long un = n;
    register int v;

    for (v = -1; un != 0; v++)
	un >>= 1;
    return v;
}



long na_lobits(n)
register long n;
{
    if (n <= 0)
        return 0;
    if (n >= 32)
        return -1;
    return (1<<n) - 1;
}



long na_hibits(n)
register long n;
{
    if (n <= 0)
        return 0;
    if (n >= 32)
        return -1;
    return ~(((unsigned long)-1) >> n);
}



long na_asl(n, x)
register long n, x;
{
    if (n > 0) {
        if (n > 31)
            return 0;
        return x << n;
    } else if (n < 0) {
        if (n < -31)
            return 0;
        VASR_L(x, -n);
        return x;
    }
    return x;
}



long na_lsl(n, x)
register long n, x;
{
    if (n > 0) {
        if (n > 31)
            return 0;
        return x << n;
    } else if (n < 0) {
        if (n < -31)
            return 0;
        return (unsigned long)x >> (-n);
    }
    return x;
}



long na_rol(n, x)
register long n, x;
{
    return ROL_L(x, n & 31);
}




/* Bit fields */

/* Bits are numbered with the MSB the longword at *v at zero. */
/* The bit field stretches from offset to (offset+size-1). */
/* This is similar to the MC68020's bit field numbering scheme. */
/* Size and offset may be any long integer values. */

#define BFCHECK(v,offset) {                 \
    if (((long)(v)) & 3) {                  \
        (offset) += 8 * (((long)(v)) & 3);  \
        (v) = (long *)(((long)(v)) & ~3);   \
    }                                       \
    v += ASR_L(offset, 5);                  \
    offset &= 31;                           \
}


/* Clear a bit-field to zeros */
Void na_bfclr(v, offset, size)
register long *v, offset, size;
{
    if (size <= 0) return;
    BFCHECK(v, offset);
    offset = 32 - offset;
    if (size <= offset) {
        *v &= ~(((1 << size) - 1) << (offset-size));
        return;
    }
    *v++ &= (-1) << offset;
    size -= offset;
    while (size >= 32) {
        *v++ = 0;
        size -= 32;
    }
    if (size)
        *v &= (1 << (32-size)) - 1;
}



/* Set a bit-field to ones */
Void na_bfset(v, offset, size)
register long *v, offset, size;
{
    if (size <= 0) return;
    BFCHECK(v, offset);
    offset = 32 - offset;
    if (size <= offset) {
        *v |= (((1 << size) - 1) << (offset-size));
        return;
    }
    *v++ |= (1 << offset) - 1;
    size -= offset;
    while (size >= 32) {
        *v++ = -1;
        size -= 32;
    }
    if (size)
        *v |= (-1) << (32-size);
}



/* Toggle the bits in a bit-field */
Void na_bfchg(v, offset, size)
register long *v, offset, size;
{
    if (size <= 0) return;
    BFCHECK(v, offset);
    offset = 32 - offset;
    if (size <= offset) {
        *v ^= (((1 << size) - 1) << (offset-size));
        return;
    }
    *v++ ^= (1 << offset) - 1;
    size -= offset;
    while (size >= 32) {
        *v++ ^= -1;
        size -= 32;
    }
    if (size)
        *v ^= (-1) << (32-size);
}



/* Test if a bit-field contains any ones */
boolean na_bftst(v, offset, size)
register long *v, offset, size;
{
    if (size <= 0) return 0;
    BFCHECK(v, offset);
    offset = 32 - offset;
    if (size <= offset)
        return (*v & (((1 << size) - 1) << (offset-size))) != 0;
    if (*v++ & ((1 << offset) - 1))
        return 1;
    size -= offset;
    while (size >= 32) {
        if (*v++)
            return 1;
        size -= 32;
    }
    return (size && (*v & ((-1) << (32-size))));
}



/* Set or clear a bit-field according to a boolean value */
Void na_bffill(v, offset, size, b)
long *v, offset, size;
boolean b;
{
    if (b)
        na_bfset(v, offset, size);
    else
        na_bfclr(v, offset, size);
}



/* Move a bit-field into another bit-field */
Void na_bfmove(s, soff, d, doff, size)
register long *s, soff, *d, doff, size;
{
    if (size <= 0) return;
    BFCHECK(d, doff);
    doff = 32 - doff;
    if (size <= doff) {
        doff -= size;
        *d &= ~(((1 << size) - 1) << doff);
        *d |= na_bfextu(s, soff, size) << doff;
        return;
    }
    *d &= (-1) << doff;
    *d++ |= na_bfextu(s, soff, doff);
    soff += doff;
    size -= doff;
    while (size >= 32) {
        *d++ = na_bfextu(s, soff, 32);
        soff += 32;
        size -= 32;
    }
    if (size) {
        *d &= (1 << (32-size)) - 1;
        *d |= na_bfextu(s, soff, size) << (32-size);
    }
}



/* Bitwise AND two bit fields */
Void na_bfand(s, soff, d, doff, size)
register long *s, soff, *d, doff, size;
{
    if (size <= 0) return;
    BFCHECK(d, doff);
    doff = 32 - doff;
    if (size <= doff) {
        doff -= size;
        *d &= (~(((1 << size) - 1) << doff)) |
              (na_bfextu(s, soff, size) << doff);
        return;
    }
    *d++ &= ((-1) << doff) | na_bfextu(s, soff, doff);
    soff += doff;
    size -= doff;
    while (size >= 32) {
        *d++ &= na_bfextu(s, soff, 32);
        soff += 32;
        size -= 32;
    }
    if (size)
        *d &= ((1 << (32-size)) - 1) |
              (na_bfextu(s, soff, size) << (32-size));
}



/* Bitwise AND-NOT two bit fields */
Void na_bfbic(s, soff, d, doff, size)
register long *s, soff, *d, doff, size;
{
    if (size <= 0) return;
    BFCHECK(d, doff);
    doff = 32 - doff;
    if (size <= doff) {
        doff -= size;
        *d &= ~(na_bfextu(s, soff, size) << doff);
        return;
    }
    *d++ &= ~na_bfextu(s, soff, doff);
    soff += doff;
    size -= doff;
    while (size >= 32) {
        *d++ &= ~na_bfextu(s, soff, 32);
        soff += 32;
        size -= 32;
    }
    if (size)
        *d &= ~(na_bfextu(s, soff, size) << (32-size));
}



/* Bitwise OR two bit fields */
Void na_bfor(s, soff, d, doff, size)
register long *s, soff, *d, doff, size;
{
    if (size <= 0) return;
    BFCHECK(d, doff);
    doff = 32 - doff;
    if (size <= doff) {
        doff -= size;
        *d |= (na_bfextu(s, soff, size) << doff);
        return;
    }
    *d++ |= na_bfextu(s, soff, doff);
    soff += doff;
    size -= doff;
    while (size >= 32) {
        *d++ |= na_bfextu(s, soff, 32);
        soff += 32;
        size -= 32;
    }
    if (size)
        *d |= (na_bfextu(s, soff, size) << (32-size));
}



/* Bitwise XOR two bit fields */
Void na_bfxor(s, soff, d, doff, size)
register long *s, soff, *d, doff, size;
{
    if (size <= 0) return;
    BFCHECK(d, doff);
    doff = 32 - doff;
    if (size <= doff) {
        doff -= size;
        *d ^= (na_bfextu(s, soff, size) << doff);
        return;
    }
    *d++ ^= na_bfextu(s, soff, doff);
    soff += doff;
    size -= doff;
    while (size >= 32) {
        *d++ ^= na_bfextu(s, soff, 32);
        soff += 32;
        size -= 32;
    }
    if (size)
        *d ^= (na_bfextu(s, soff, size) << (32-size));
}



/* Reverse a bit field LSB-for-MSB */
Void na_bfrev(v, offset, size)
register long *v, offset, size;
{
    if (size <= 1) return;
    if (size <= 32) {
        register long val, val2 = 0;
        val = na_bfextu(v, offset, size);
        while (--size >= 0) {
            val2 <<= 1;
            val2 += (val & 1);
            val >>= 1;
        }
        na_bfins(v, offset, size, val2);
        return;
    }
    BFCHECK(v, offset);
    {   register long *v2, mask1, mask2;
        mask1 = 1 << (31-offset);
        offset += size - 1;
        v2 = v + ASR_L(offset, 5);
        mask2 = 1 << ((~offset) & 31);
        size >>= 1;
        while (--size >= 0) {
            if (*v & mask1) {
                if (!(*v2 & mask2)) {
                    *v ^= mask1;
                    *v2 ^= mask2;
                }
            } else {
                if (*v2 & mask2) {
                    *v ^= mask1;
                    *v2 ^= mask2;
                }
            }
            if (mask1 == 1) {
                mask1 = 0x80000000;
                v++;
            } else
                mask1 >>= 1;
            if (mask2 < 0) {
                mask2 = 1;
                v2--;
            } else
                mask2 <<= 1;
        }
    }
}



/* Insert a value (up to 32 bits) into a bit-field */
Void na_bfins(v, offset, size, val)
register long *v, offset, size, val;
{
    register long mask;

    if (size <= 0) return;
    BFCHECK(v, offset);
    offset = 32 - offset;
    if (size <= offset) {
        offset -= size;
        mask = (((1 << size) - 1) << offset);
        *v &= ~mask;
        *v |= (val << offset) & mask;
        return;
    }
    mask = (1 << offset) - 1;
    *v &= ~mask;
    size -= offset;
    *v++ |= ((unsigned long)val >> (size)) & mask;
    mask = (-1) << (32-size);
    *v &= ~mask;
    *v |= val << (32-size);
}



/* Extract an unsigned value (up to 32 bits) from a bit-field */
long na_bfextu(v, offset, size)
register long *v, offset, size;
{
    register long val;

    if (size <= 0) return 0;
    BFCHECK(v, offset);
    offset = 32 - offset;
    if (size <= offset)
        return ((*v) >> (offset-size)) &
               ((1 << size) - 1);
    size -= offset;
    val = ((*v++) & ((1 << offset) - 1)) << size;
    return val | ((unsigned long)(*v)) >> (32-size);
}



/* Extract a signed value (up to 32 bits) from a bit-field */
long na_bfexts(v, offset, size)
register long *v, offset, size;
{
    register long val;

    if (size <= 0) return 0;
    BFCHECK(v, offset);
    if (size <= 32 - offset)
        return ASR_L((*v) << offset, 32-size);
    val = (*v++) << offset;
    val = ASR_L(val, 32-size);
    return val | ((unsigned long)(*v)) >> (64-size-offset);
}



/* Find first (left-most) 'one' bit in a bit-field */
long na_bfffo(v, offset, size)
register long *v, offset, size;
{
    register long val;

    if (size <= 0) return offset;
    while (size >= 32) {
        val = na_bfextu(v, offset, 32);
        if (val) {
            if (val < 0x10000) {
                offset += 16;
                val <<= 16;
            }
            while (val >= 0) {
                offset++;
                val <<= 1;
            }
            return offset;
        }
        offset += 32;
        size -= 32;
    }
    val = na_bfextu(v, offset, size);
    if (val) {
        val <<= 32-size;
        while (val >= 0) {
            offset++;
            val <<= 1;
        }
        return offset;
    }
    return offset + size;
}



/* Find first (left-most) 'zero' bit in a bit-field */
long na_bfffz(v, offset, size)
register long *v, offset, size;
{
    register long val;

    if (size <= 0) return offset;
    while (size >= 32) {
        val = na_bfextu(v, offset, 32);
        if (~val) {
            if ((unsigned long)val >= 0xffff0000) {
                offset += 16;
                val <<= 16;
            }
            while (val < 0) {
                offset++;
                val <<= 1;
            }
            return offset;
        }
        offset += 32;
        size -= 32;
    }
    val = na_bfextu(v, offset, size);
    if (~val) {
        val <<= 32-size;
        while (val < 0) {
            offset++;
            val <<= 1;
        }
        return offset;
    }
    return offset + size;
}



/* Count the number of 'one' bits in a bit-field */
long na_bfcount(v, offset, size)
register long *v, offset, size;
{
    register long val, count = 0;

    if (size <= 0) return 0;
    while (size >= 32) {
        val = na_bfextu(v, offset, 32);
        if (~val) {
            while (val) {
                count++;
                val &= (val - 1);
            }
        } else
            count += 32;
        offset += 32;
        size -= 32;
    }
    val = na_bfextu(v, offset, size);
    while (val) {
        count++;
        val &= (val - 1);
    }
    return count;
}




/* Operations on Pascal sets */

/* Cardinality of a set */
long na_setcard(s)
long *s;
{
    return na_bfcount(s+1, 0, *s << 5);
}



/* Highest ordinal value in set */
long na_setlast(s)
long *s;
{
    register long count, val;

    if (!*s)
        return -1;
    count = *s - 1;
    val = s[count];
    while (!val) {    /* should not happen */
        if (--count < 0)
            return -1;
        val = s[count];
    }
    count <<= 5;
    while (val >= 0) {
        count++;
        val <<= 1;
    }
    return count;
}


/* Next value in set after a certain value */
long na_setnext(s, offset)
long *s, offset;
{
    long val;

    if (offset < 0) offset = 0;
    val = na_bfffo(s+1, offset, (*s << 5) - offset);
    if (val < (*s << 5))
        return val;
    else
        return -1;
}



/* Next value not in set after a certain value */
long na_setnextnot(s, offset)
long *s, offset;
{
    long val;

    if (offset < 0) return offset;
    val = na_bfffz(s+1, offset, (*s << 5) - offset);
    if (val < (*s << 5))
        return val;
    else
        return offset;
}




/* Miscellaneous functions */

double P_rmin(a, b)
double a, b;
{
    if (a < b)
        return a;
    else
        return b;
}



double P_rmax(a, b)
double a, b;
{
    if (a > b)
        return a;
    else
        return b;
}



long P_rsgn(a)
double a;
{
    if (a < 0)
        return -1;
    else if (a > 0)
        return 1;
    else
        return 0;
}



long P_isgn(a)
register long a;
{
    if (a < 0)
        return -1;
    else if (a > 0)
        return 1;
    else
        return 0;
}



long P_imin2(a, b)
register long a, b;
{
    if (a < b)
        return a;
    else
        return b;
}



long P_imax2(a, b)
register long a, b;
{
    if (a > b)
        return a;
    else
        return b;
}



long P_imin3(a, b, c)
register long a, b, c;
{
    if (a < b)
        return (a < c) ? c : a;
    else
        return (b < c) ? c : b;
}



long P_imax3(a, b, c)
register long a, b, c;
{
    if (a > b)
        return (a > c) ? c : a;
    else
        return (b > c) ? c : b;
}



long P_imin4(a, b, c, d)
register long a, b, c, d;
{
    if (a < b)
        if (a < c)
            return (a < d) ? d : a;
        else
            return (c < d) ? d : c;
    else
        if (b < c)
            return (b < d) ? d : b;
        else
            return (c < d) ? d : c;
}



long P_imax4(a, b, c, d)
register long a, b, c, d;
{
    if (a > b)
        if (a > c)
            return (a > d) ? d : a;
        else
            return (c > d) ? d : c;
    else
        if (b > c)
            return (b > d) ? d : b;
        else
            return (c > d) ? d : c;
}



Void P_iswap2(a, b)
register long *a, *b;
{
    register long t;

    t = *a;
    *a = *b;
    *b = t;
}



Void P_isort2(a, b)
register long *a, *b;
{
    register long t;

    t = *a;
    if (t > *b) {
        *a = *b;
        *b = t;
    }
}



Void P_isort3(a, b, c)
register long *a, *b, *c;
{
    register long t;

    t = *a;
    if (t < *b) {                   /* a < b */
        if (*c < t) {               /* c < a < b */
            *a = *c;
            *c = *b;
            *b = t;
        } else if (*c < *b) {       /* a <= c < b */
            t = *b;
            *b = *c;
            *c = t;
        }
    } else {                        /* b <= a */
        if (*c < *b) {              /* c < b <= a */
            *a = *c;
            *c = t;
        } else if (*c < t) {        /* b <= c < a */
            *a = *b;
            *b = *c;
            *c = t;
        } else {                    /* b <= a <= c */
            *a = *b;
            *b = t;
        }
    }
}



Void P_isort4(a, b, c, d)
register long *a, *b, *c, *d;
{
    register long t;

    P_isort3(a, b, c);
    t = *d;
    if (t < *a) {               /* d < a <= b <= c */
        *d = *c;
        *c = *b;
        *b = *a;
        *a = t;
    } else if (t < *b) {        /* a <= d < b <= c */
        *d = *c;
        *c = *b;
        *b = t;
    } else if (t < *c) {        /* a <= b <= d < c */
        *d = *c;
        *c = t;
    }
}



long P_ibound(min, i, max)
register long min, i, max;
{
    if (max > min)
        return (i < min) ? min : (i > max) ? max : i;
    else
        return (i < max) ? max : (i > min) ? min : i;
}



boolean P_ibetween(min, i, max)
register long min, i, max;
{
    if (min <= i)
        return (i <= max);
    else
        return (max <= i);
}



boolean P_ibetweenm(min, i, max, margin)
register long min, i, max, margin;
{
    return ((min-margin <= i) && (i <= max+margin)) ||
           ((max-margin <= i) && (i <= min+margin));
}



Void P_iamin(min, i)
register long *min, i;
{
    if (i < *min)
        *min = i;
}



Void P_iamax(max, i)
register long *max, i;
{
    if (i > *max)
        *max = i;
}



Void P_iaminmax(min, max, i)
register long *min, *max, i;
{
    if (i < *min)
        *min = i;
    if (i > *max)
        *max = i;
}



long P_isqrt(n)
long n;
{
    return (long)sqrt((double)n);      /* ought to use a better algorithm for this! */
}



long P_ipow(y, x)
register long y, x;
{
    register long n;

    switch (y) {

        case 0:
        case 1:
            return y;

        case 2:
            return (x >= 0) ? 1 << x : 0;

        case -1:
            return (x & 1) ? -1 : 1;

        case -2:
            return (x & 1) ? -(1 << x) : (1 << x);

    }
    switch (x) {

        case 0:
            return 1;

        case 1:
            return y;

    }
    if (x < 0)
        return 0;
    if (y < 0) {
        y = - y;
        if (x & 1)
            return -P_ipow(y, x);
    }
    n = (x & 1) ? y : 1;
    for (;;) {
        x >>= 1;
        if (!x)
            return n;
        y *= y;
        if (x & 1)
            n *= y;
    }
}



Void na_addc(a, b, c)
long *a, b;
boolean *c;
{
    *a += b + *c;
    *c = ((unsigned long)*a < (unsigned long)b);
}



Void na_subc(a, b, c)
long *a, b;
boolean *c;
{
    boolean newc = ((unsigned long)*a < (unsigned long)b);

    *a -= b - *c;
    *c = newc;
}



long na_smul(a, b)
long a, b;
{
    return (unsigned long)a * (unsigned long)b;
}



long na_umul(a, b)
long a, b;
{
    return (unsigned long)a * (unsigned long)b;
}



na_quadword na_smul64(a, b)
long a, b;
{
    na_quadword q;

    if ((a ^ b) < 0) {
        if (a < 0)
            a = -a;
        else
            b = -b;
        q = na_umul64(a, b);
        q.U1.i2 = - q.U1.i2;
        q.U1.i1 = - q.U1.i1 - (q.U1.i2 != 0);
    }
    if (a < 0) {
        a = -a;
        b = -b;
    }
    return na_umul64(a, b);
}



na_quadword na_umul64(a, b)
long a, b;
{
    na_quadword q;
    unsigned short ah = a >> 16, al = a;
    unsigned short bh = b >> 16, bl = b;
    boolean c;
    unsigned long t;

    q.U1.i1 = al * bl;
    t = bh * al;
    c = 0;
    na_addc(&q.U1.i1, t << 16, &c);
    q.U1.i2 = ah * bh + (t >> 16) + c;
    t = ah * bl;
    c = 0;
    na_addc(&q.U1.i1, t << 16, &c);
    q.U1.i2 += (t >> 16) + c;
    return q;
}




/* Quick-sort */

static int (*userqsortfunc)(long, long, ...);
static Anyptr userqsortlink;

static int qsortfunc(const void *a, const void *b)
{
    return (*userqsortfunc)(*(const long *)a, *(const long *)b);
}

static int qsortfunc2(const void *a, const void *b)
{
    return (*userqsortfunc)(*(const long *)a, *(const long *)b, userqsortlink);
}

Void na_qsort(a, n, comp)
na_long *a;
long n;
_PROCEDURE comp;
{
    if (comp.link)
      {
        userqsortfunc = (int (*)(long, long, ...))comp.proc;
        userqsortlink = comp.link;

#ifndef OS2
	qsort(a, n, 4, qsortfunc2);
#endif

      }
     else
       {
	 userqsortfunc = (int (*)(long, long, ...))comp.proc;

#ifndef OS2
	 qsort(a, n, 4, qsortfunc);
#endif
	 
       }
  }





/* String operations */

Void strsafeappend(s, s2)
register Char *s, *s2;
{
    register Char *cp;

    cp = s;
    while (*cp) cp++;
    s += 255;
    while (cp < s && *s2)
        *cp++ = *s2++;
    *cp = 0;
}



Char *strupper(res, s)
register Char *res, *s;
{
    register Char *d = res;
    while (*s)
        *d++ = toupper(*s++);
    *d = 0;
    return res;
}



Char *strlower(res, s)
register Char *res, *s;
{
    register Char *d = res;
    while (*s)
        *d++ = tolower(*s++);
    *d = 0;
    return res;
}



Char *strpart(res, s, first, last)
Char *res, *s;
long first, last;
{
    return strsub(res, s, first, last-first+1);
}



Char *strlast(res, s, num)
Char *res, *s;
long num;
{
    register Char *cp;

    cp = s + strlen(s) - num;
    if (cp <= s)
        return strcpy(res, s);
    else
        return strcpy(res, cp);
}



Char *strcjust(res, s, wid)
register Char *res, *s;
register long wid;
{
    register int len = strlen(s), wid2;
    register char *cp = res;

    if (wid > 255)
        wid = 255;
    wid -= len;
    if (wid <= 0)
        return strcpy(res, s);
    wid2 = wid >> 1;
    while (--wid2 >= 0)
        *cp++ = ' ';
    while (*s)
        *cp++ = *s++;
    wid2 = (wid + 1) >> 1;
    while (--wid2 >= 0)
        *cp++ = ' ';
    *cp = 0;
    return res;
}



boolean strbegins(s1, s2)
register Char *s1, *s2;
{
    while (*s2)
        if (*s1++ != *s2++)
            return 0;
    return 1;
}



boolean strends(s1, s2)
register Char *s1, *s2;
{
    register int skip = strlen(s1) - strlen(s2);

    if (skip < 0)
        return 0;
    return (strcmp(s1 + skip, s2) == 0);
}



boolean strcibegins(s1, s2)
register Char *s1, *s2;
{
    while (*s2)
        if (isalpha(*s1) ? (toupper(*s1++) != toupper(*s2++))
                         : (*s1++ != *s2++))
            return 0;
    return 1;
}



boolean strciends(s1, s2)
register Char *s1, *s2;
{
    register int skip = strlen(s1) - strlen(s2);

    if (skip < 0)
        return 0;
    return !strcicmp(s1 + skip, s2);
}



#if 0   /* now in p2clib.c */
int strcicmp(s1, s2)
register Char *s1, *s2;
{
    register Char ch;

    while (*s2)
        if (isalpha(ch = *s1++) ? (toupper(ch) != toupper(*s2++))
                         : (ch != *s2++))
            return (toupper(ch) > toupper(*--s2)) ? 1 : -1;
    return (*s1 == 0) ? 0 : 1;
}
#endif



long strsubcmp(n, s1, i1, s2, i2)
register Char *s1, *s2;
register long n, i1, i2;
{
    if (i1 <= 0) s1 = (Char *)"";
    if (i2 <= 0) s2 = (Char *)"";
    while (--i1 > 0) {
        if (!*s1++) {
            s1--;
            break;
        }
    }
    while (--i2 > 0) {
        if (!*s2++) {
            s2--;
            break;
        }
    }
    return strncmp(s1, s2, (int)n);
}



boolean strcisubcmp(n, s1, i1, s2, i2)
register Char *s1, *s2;
register long n, i1, i2;
{
    if (i1 <= 0) s1 = (Char *)"";
    if (i2 <= 0) s2 = (Char *)"";
    while (--i1 > 0) {
        if (!*s1++) {
            s1--;
            break;
        }
    }
    while (--i2 > 0) {
        if (!*s2++) {
            s2--;
            break;
        }
    }
    while (--n >= 0) {
        if (toupper(*s1++) != toupper(*s2++))
            return 0;
    }
    return 1;
}



Void strsubst(repl, s, index, size)
register Char *repl, *s;
register long index, size;
{
    register int len = strlen(repl);

    s += index - 1;
    size -= len;
    if (size < 0) {
        register Char *cp;
        for (cp = s + strlen(s); cp >= s; cp--)
            *cp = cp[size];
    } else if (size > 0) {
        register Char *cp;
        for (cp = s; (*cp = cp[size]); cp++) ;
    }
    strncpy(s, repl, len);
}



Char *strint2(res, i, w)
Char *res;
long i, w;
{
    if (w < 0)
        sprintf(res, "%0*ld", (int)(-w), i);
    else
        sprintf(res, "%*ld", (int)w, i);
    return res;
}



long strposb(s1, s2, i)
register Char *s1, *s2;
register long i;
{
    register int len1, len2;

    len1 = strlen(s1);
    len2 = strlen(s2);
    if (--i > len1 - len2)
        i = len1 - len2;
    if (i < 0 || !len2)
        return 0;
    while (i >= 0 && strncmp(s1 + i, s2, len2))
        i--;
    return i + 1;
}



long strposc(s1, ch, i)
register Char *s1, ch;
register long i;
{
    register Char *cp = s1;

    if (i <= 0)
        return 0;
    while (--i > 0)
        if (!*cp++)
            return 0;
    while (*cp)
        if (*cp++ == ch)
            return cp - s1;
    return 0;
}



long strcipos2(s1, s2, i)
register Char *s1, *s2;
register long i;
{
    register Char *cp = s1;
    register int len = strlen(s2);

    if (i <= 0 || !len)
        return 0;
    while (--i > 0)
        if (!*cp++)
            return 0;
    while (*cp)
        if (!strncmp(cp++, s2, len))
            return cp - s1;
    return 0;
}



long strciposb(s1, s2, i)
register Char *s1, *s2;
register long i;
{
    register int len;

    len = strlen(s1) - strlen(s2);
    if (--i > len)
        i = len;
    if (i < 0 || !*s2)
        return 0;
    while (i >= 0 && !strcibegins(s1 + i, s2))
        i--;
    return i + 1;
}



long strposcset(s1, s2, i)
register Char *s1, *s2;
register long i;
{
    register Char *cp = s1, *cp2;
    register Char ch, ch2;

    if (i <= 0)
        return 0;
    while (--i > 0)
        if (!*cp++)
            return 0;
    while ((ch = *cp++)) {
        for (cp2 = s2; (ch2 = *cp2); cp2++)
            if (ch == ch2)
                return cp - s1;
    }
    return 0;
}



long strposbcset(s1, s2, i)
register Char *s1, *s2;
register long i;
{
    register Char *cp2;
    register Char ch, ch2;
    register int len;

    len = strlen(s1);
    if (i >= len)
        i = len;
    if (i <= 0)
        return 0;
    while (--i >= 0) {
        ch = s1[i];
        for (cp2 = s2; (ch2 = *cp2); cp2++)
            if (ch == ch2)
                return i + 1;
    }
    return 0;
}



boolean strsubset(s1, s2)
register Char *s1, *s2;
{
    register Char *cp, ch, ch2;

    while ((ch = *s1++)) {
        for (cp = s2; (ch2 = *s2++) != ch; cp++)
            if (!ch2)
                return 0;
    }
    return 1;
}



boolean strcommon(s1, s2)
register Char *s1, *s2;
{
    return strsubset(s1, s2) && strsubset(s2, s1);
}



long strlenv(s1)
register Char *s1;
{
    register int len = 0;
    register Char ch;

    while ((ch = *s1++)) {
        switch (ch) {

            case 0:
            case 7:
                break;

            case 8:
                if (--len < 0)
                    ++len;
                break;

            case 9:
                len = (len | 7) + 1;
                break;

            case 12:
            case 13:
                len = 0;
                break;

            default:
                ++len;
        }
    }
    return len;
}



Char *strhex(res, i, w)
Char *res;
long i, w;
{
    if (w < 1 || w > 8)
        sprintf(res, "%lX", i);
    else
        sprintf(res, "%.*lX", (int)w, i);
    return res;
}



Char *strbinary(res, i, w)
Char *res;
register long i, w;
{
    if (w < 1 || w > 32) {
        if (i) {
            register long i2 = i;
            w = 32;
            while (i2 >= 0) {
                w--;
                i2 <<= 1;
            }
        } else
            w = 1;
    }
    res[w] = 0;
    while (w > 0) {
        --w;
        res[w] = '0' + (i & 1);
        i >>= 1;
    }
    return res;
}



Char *strdate(res, opts)    /* ignore opts for now */
Char *res, *opts;
{
    time_t t = time((long *)0);
    strcpy(res, (char *) ctime(&t));
    res[strlen(res)-1] = 0;    /* eat the \n */
    return res;
}



Void strword(s, word)
register Char *s, *word;
{
    register Char *s1 = s;

    while (isspace(*s1)) s1++;
    while (*s1 && !isspace(*s1))
        *word++ = *s1++;
    while (isspace(*s1)) s1++;
    *word = 0;
    while (*s1)
        *s++ = *s1++;
    *s = 0;
}



Void strword2(s, word)
register Char *s, *word;
{
    register Char *s1 = s;
    register Char ch;

    while (isspace(*s1)) s1++;
    if (*s1 == '\'' || *s1 == '"') {
        ch = *s1++;
        while (*s1 && *s1 != ch)
            *word++ = *s1++;
        if (*s1 == ch)
            s1++;
    } else {
        while (*s1 && !isspace(*s1))
            *word++ = *s1++;
    }
    while (isspace(*s1)) s1++;
    *word = 0;
    while (*s1)
        *s++ = *s1++;
    *s = 0;
}



Void strcomment(s, bracket)
register Char *s;
Char *bracket;
{
    register Char *cp;
    register char ch1, ch2;

    ch1 = bracket[0];
    ch2 = bracket[1];
    while (*s && *s != ch1) s++;
    if (*s) {
        cp = s;
        while (*s) {
            if (*s == ch1) {
                if (ch2) {
                    s++;
                    while (*s && *s != ch2) s++;
                    if (*s)
                        s++;
                } else
                    break;
            } else
                *cp++ = *s++;
        }
        *cp = 0;
    }
}



Void strcompress(s, chars, all)
register Char *s, *chars;
boolean all;
{
    register Char *cp = s, *cp2;
    register Char ch;

    while ((ch = *s)) {
        for (cp2 = chars; *cp2 && *cp2 != ch; cp2++) ;
        if (*cp2) {
            if (!all)
                *cp++ = ch;
            do {
                if (!(ch = *++s))
                    break;
                for (cp2 = chars; *cp2 && *cp2 != ch; cp2++) ;
            } while (*cp2);
        } else {
            *cp++ = ch;
            s++;
        }
    }
    *cp = 0;
}





/* Variable-length string allocation */

static Void outmem()
{
    fprintf(stderr, "Memory full!\n");
    exit(1);
}


#ifdef LACKS_STRDUP
Char *strdup(s)
Char *s;
{
    Char *sp;
    int len = strlen(s);

    sp = (Char *)malloc(len + 1);
    if (!sp) outmem();
    strcpy(sp, s);
    return sp;
}
#endif



Void strdispose(sp)
Char **sp;
{
    if (*sp)
        free(*sp);
    *sp = NULL;
}



Void strchange(sp, s)
Char **sp, *s;
{
    int len1, len2 = strlen(s);
    if (*sp) {
        len1 = strlen(*sp);
        if (len1 != len2)
            *sp = (Char *)realloc(*sp, len2 + 1);
    } else
        *sp = (Char *)malloc(len2 + 1);
    if (!*sp) outmem();
    strcpy(*sp, s);
}







/* String lists */


Void strlist_init(base)
na_strlist **base;
{
    *base = NULL;
}



na_strlist *strlist_add(base, s)
register na_strlist **base;
register Char *s;
{
    register na_strlist *lp;
    register int len = strlen(s);

    while ((lp = *base) && strcmp(lp->s, s) < 0)
        base = &lp->next;
    if (!lp || strcmp(lp->s, s)) {
#ifdef __alpha__
        lp = (na_strlist *)malloc(sizeof(na_strlistrec));
#else
        lp = (na_strlist *)malloc(sizeof(na_strlistrec) - 255 + len);
#endif
        if (!lp) outmem();
        lp->next = *base;
        *base = lp;
        strcpy(lp->s, s);
        lp->len = len;
    }
    lp->value = (na_long)0;
    lp->kind = 0;
    return lp;
}



na_strlist *strlist_append(base, s)
register na_strlist **base;
register Char *s;
{
    register na_strlist *lp;
    register int len = strlen(s);

    while ((lp = *base))
        base = &lp->next;
#ifdef __alpha__
    lp = (na_strlist *)malloc(sizeof(na_strlistrec));
#else
    lp = (na_strlist *)malloc(sizeof(na_strlistrec) - 255 + len);
#endif
    if (!lp) outmem();
    lp->next = NULL;
    *base = lp;
    strcpy(lp->s, s);
    lp->len = len;
    lp->value = (na_long)0;
    lp->kind = 0;
    return lp;
}



na_strlist *strlist_insert(base, s)
register na_strlist **base;
register Char *s;
{
    register na_strlist *lp;
    register int len = strlen(s);

#ifdef __alpha__
    lp = (na_strlist *)malloc(sizeof(na_strlistrec));
#else
    lp = (na_strlist *)malloc(sizeof(na_strlistrec) - 255 + len);
#endif
    if (!lp) outmem();
    lp->next = *base;
    *base = lp;
    strcpy(lp->s, s);
    lp->len = len;
    lp->value = (na_long)0;
    lp->kind = 0;
    return lp;
}



na_strlist *strlist_find(base, s)
register na_strlist *base;
register Char *s;
{
    while (base && strcmp(base->s, s))
        base = base->next;
    return base;
}



na_strlist *strlist_cifind(base, s)
register na_strlist *base;
register Char *s;
{
    while (base && strcicmp(base->s, s))
        base = base->next;
    return base;
}



Void strlist_change(base, lp, s)
register na_strlist **base, **lp;
register Char *s;
{
    register na_strlist *lp2 = *lp, *lp3;
    register int len = strlen(s);

    while ((lp3 = *base) != lp2) {
        if (lp3)
            base = &lp3->next;
        else
            return;   /* wasn't in the list */
    }
    if (len > lp3->len) {       /* must reallocate */
        lp3 = (na_strlist *)realloc(lp3, sizeof(na_strlistrec) - 255 + len);
        if (!lp3) outmem();
        lp3->len = len;
        *lp = lp3;
        lp3->next = *base;
        *base = lp3;
    }
    strcpy(lp3->s, s);
}



Void strlist_remove(base, s)
register na_strlist **base;
register Char *s;
{
    register na_strlist *lp;

    while ((lp = *base)) {
        if (!strcmp(lp->s, s)) {
            *base = lp->next;
            free(lp);
            return;
        }
        base = &lp->next;
    }
}



Void strlist_delete(base, lp)
register na_strlist **base, *lp;
{
    register na_strlist *lp2;

    while ((lp2 = *base)) {
        if (lp2 == lp) {
            *base = lp->next;
            free(lp);
            return;
        }
        base = &lp2->next;
    }
}



Void strlist_dispose(ptr)
register na_strlist **ptr;
{
    free(*ptr);
    *ptr = NULL;
}



Void strlist_empty(base)
register na_strlist **base;
{
    register na_strlist *lp;

    while ((lp = *base)) {
        *base = lp->next;
        free(lp);
    }
}



Void strlist_copy(dest, src)
register na_strlist **dest, *src;
{
    register na_strlist *lp;

    while (src) {
#ifdef __alpha__
        lp = (na_strlist *)malloc(sizeof(na_strlistrec));
#else
        lp = (na_strlist *)malloc(sizeof(na_strlistrec) - 255 + src->len);
#endif
        lp->len = src->len;
        lp->kind = src->kind;
        lp->value = src->value;
        strcpy(lp->s, src->s);
        *dest = lp;
        dest = &lp->next;
	src = src->next;   /*  Change made by rhkoshi without much thought  */
    }
    *dest = NULL;
}



Void strlist_dump(f, base)
register FILE *f;
register na_strlist *base;
{
    while (base) {
        fprintf(f, "\"%s\" %d, %ld\n", base->s, base->kind, (long)base->value);
        base = base->next;
    }
}



double na_srtor(x)
Anyptr x;
{
  return(*((float *) &x));
}


Anyptr na_rtosr(x)
double x;
{
  float y = x;

  return(*((Anyptr *) &y));
}

/* added in 2013, to replace strcpy calls with undefined semantics */

char * strcpy_overlap(char * dst, char * src)

{
  int i = 0;

  do {
    dst[i] = src[i];
    if (src[i++] == '\0')
      break;
  } while (1);
  return dst;
}

/* End. */


