/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995--1997 Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1998--2016 The R Core Team.
 *  Copyright (C) 2003--2016 The R Foundation
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, a copy is available at
 *  https://www.R-project.org/Licenses/
 */

/* interval at which to check interrupts, a guess */
#define NINTERRUPT 10000000


#ifdef __OpenBSD__
/* for definition of "struct exception" in math.h */
# define __LIBM_PRIVATE
#endif
//#include <Defn.h>		/*-> Arith.h -> math.h */
#ifdef __OpenBSD__
# undef __LIBM_PRIVATE
#endif

#include "Rarith.h"

double R_NaN;		/* IEEE NaN */
double R_PosInf;	/* IEEE Inf */
double R_NegInf;	/* IEEE -Inf */
double R_NaReal;	/* NA_REAL: IEEE */
int	 R_NaInt;	/* NA_INTEGER:= INT_MIN currently */

typedef union
{
    double value;
    unsigned int word[2];
} ieee_double;

/* gcc had problems with static const on AIX and Solaris
   Solaris was for gcc 3.1 and 3.2 under -O2 32-bit on 64-bit kernel */
#ifdef _AIX
#define CONST
#elif defined(sparc) && defined (__GNUC__) && __GNUC__ == 3
#define CONST
#else
#define CONST const
#endif

#ifdef WORDS_BIGENDIAN
static CONST int hw = 0;
static CONST int lw = 1;
#else  /* !WORDS_BIGENDIAN */
static CONST int hw = 1;
static CONST int lw = 0;
#endif /* WORDS_BIGENDIAN */


static double R_ValueOfNA(void)
{
    /* The gcc shipping with Fedora 9 gets this wrong without
     * the volatile declaration. Thanks to Marc Schwartz. */
    volatile ieee_double x;
    x.word[hw] = 0x7ff00000;
    x.word[lw] = 1954;
    return x.value;
}

int R_IsNA(double x)
{
    if (isnan(x)) {
	ieee_double y;
	y.value = x;
	return (y.word[lw] == 1954);
    }
    return 0;
}

int R_IsNaN(double x)
{
    if (isnan(x)) {
	ieee_double y;
	y.value = x;
	return (y.word[lw] != 1954);
    }
    return 0;
}

/* ISNAN uses isnan, which is undefined by C++ headers
   This workaround is called only when ISNAN() is used
   in a user code in a file with __cplusplus defined */

int R_isnancpp(double x)
{
   return (isnan(x)!=0);
}


/* Mainly for use in packages */
int R_finite(double x)
{
#ifdef MATH_HAVE_ISFINITE
    return isfinite(x);
#else
    return (!isnan(x) & (x != R_PosInf) & (x != R_NegInf));
#endif
}


/* Arithmetic Initialization */

void R_init()
{
    R_NaInt = INT_MIN;
    R_NaReal = R_ValueOfNA();
// we assume C99, so
#ifndef OLD
    R_NaN = NAN;
    R_PosInf = INFINITY;
    R_NegInf = -INFINITY;
#else
    R_NaN = 0.0/R_Zero_Hack;
    R_PosInf = 1.0/R_Zero_Hack;
    R_NegInf = -1.0/R_Zero_Hack;
#endif
}

/* Keep these two in step */
/* FIXME: consider using
tmp = (LDOUBLE)x1 - floor(q) * (LDOUBLE)x2;
*/
static double myfmod(double x1, double x2)
{
	if (x2 == 0.0) return R_NaN;
	double q = x1 / x2, tmp = x1 - floor(q) * x2;
	//if (R_finite(q) && (fabs(q) > 1 / R_AccuracyInfo.eps))
	//{
	//	warning(_("probable complete loss of accuracy in modulus"));
	//}
	q = floor(tmp / x2);
	return tmp - q * x2;
}

static double myfloor(double x1, double x2)
{
	double q = x1 / x2, tmp;

	if (x2 == 0.0) return q;
	tmp = x1 - floor(q) * x2;
	return floor(q) + floor(tmp / x2);
}

double R_pow(double x, double y) /* = x ^ y */
{
    /* squaring is the most common of the specially handled cases so
       check for it first. */
    if(y == 2.0)
	return x * x;
    if(x == 1. || y == 0.)
	return(1.);
    if(x == 0.) {
	if(y > 0.) return(0.);
	else if(y < 0) return(R_PosInf);
	else return(y); /* NA or NaN, we assert */
    }
    if (R_finite(x) && R_finite(y)) {
	/* There was a special case for y == 0.5 here, but
	   gcc 4.3.0 -g -O2 mis-compiled it.  Showed up with
	   100^0.5 as 3.162278, example(pbirthday) failed. */
    return pow(x, y);
    }
    if (ISNAN(x) || ISNAN(y))
	return(x + y);
    if(!R_finite(x)) {
	if(x > 0)		/* Inf ^ y */
	    return (y < 0.)? 0. : R_PosInf;
	else {			/* (-Inf) ^ y */
	    if(R_finite(y) && y == floor(y)) /* (-Inf) ^ n */
		return (y < 0.) ? 0. : (myfmod(y, 2.) != 0 ? x  : -x);
	}
    }
    if(!R_finite(y)) {
	if(x >= 0) {
	    if(y > 0)		/* y == +Inf */
		return (x >= 1) ? R_PosInf : 0.;
	    else		/* y == -Inf */
		return (x < 1) ? R_PosInf : 0.;
	}
    }
    return R_NaN; // all other cases: (-Inf)^{+-Inf, non-int}; (neg)^{+-Inf}
}

double R_pow_di(double x, int n)
{
    double xn = 1.0;

    if (ISNAN(x)) return x;
    if (n == NA_INTEGER) return NA_REAL;

    if (n != 0) {
	if (!R_finite(x)) return R_pow(x, (double)n);

	int is_neg = (n < 0);
	if(is_neg) n = -n;
	for(;;) {
	    if(n & 01) xn *= x;
	    if(n >>= 1) x *= x; else break;
	}
	if(is_neg) xn = 1. / xn;
    }
    return xn;
}



/* Integer arithmetic support */

/* The tests using integer comparisons are a bit faster than the tests
   using doubles, but they depend on a two's complement representation
   (but that is almost universal).  The tests that compare results to
   double's depend on being able to accurately represent all int's as
   double's.  Since int's are almost universally 32 bit that should be
   OK. */


#define INTEGER_OVERFLOW_WARNING _("NAs produced by integer overflow")

#define CHECK_INTEGER_OVERFLOW(call, ans, naflag) do {		\
	if (naflag) {						\
	    PROTECT(ans);					\
	    warningcall(call, INTEGER_OVERFLOW_WARNING);	\
	    UNPROTECT(1);					\
	}							\
    } while(0)

#define R_INT_MAX  INT_MAX
#define R_INT_MIN -INT_MAX
// .. relying on fact that NA_INTEGER is outside of these

static int R_integer_plus(int x, int y, int *pnaflag)
{
    if (x == NA_INTEGER || y == NA_INTEGER)
	return NA_INTEGER;

    if (((y > 0) && (x > (R_INT_MAX - y))) ||
	((y < 0) && (x < (R_INT_MIN - y)))) {
	if (pnaflag != NULL)
	    *pnaflag = 1;
	return NA_INTEGER;
    }
    return x + y;
}

static int R_integer_minus(int x, int y, int *pnaflag)
{
    if (x == NA_INTEGER || y == NA_INTEGER)
	return NA_INTEGER;

    if (((y < 0) && (x > (R_INT_MAX + y))) ||
	((y > 0) && (x < (R_INT_MIN + y)))) {
	if (pnaflag != NULL)
	    *pnaflag = 1;
	return NA_INTEGER;
    }
    return x - y;
}

#define GOODIPROD(x, y, z) ((double) (x) * (double) (y) == (z))
static int R_integer_times(int x, int y, int *pnaflag)
{
    if (x == NA_INTEGER || y == NA_INTEGER)
	return NA_INTEGER;
    else {
	int z = x * y;  // UBSAN will warn if this overflows (happens in bda)
	if (GOODIPROD(x, y, z) && z != NA_INTEGER)
	    return z;
	else {
	    if (pnaflag != NULL)
		*pnaflag = 1;
	    return NA_INTEGER;
	}
    }
}

static double R_integer_divide(int x, int y)
{
    if (x == NA_INTEGER || y == NA_INTEGER)
	return NA_REAL;
    else
	return (double) x / (double) y;
}

