﻿/*
 *  Mathlib : A C Library of Special Functions
 *  Copyright (C) 2005-6 Morten Welinder <terra@gnome.org>
 *  Copyright (C) 2005-10 The R Foundation
 *  Copyright (C) 2006-2015 The R Core Team
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
 *
 *  SYNOPSIS
 *
 *	#include <Rmath.h>
 *
 *	double pgamma (double x, double alph, double scale,
 *		       int lower_tail, int log_p)
 *
 *	double log1pmx	(double x)
 *	double lgamma1p (double a)
 *
 *	double logspace_add (double logx, double logy)
 *	double logspace_sub (double logx, double logy)
 *	double logspace_sum (double* logx, int n)
 *
 *
 *  DESCRIPTION
 *
 *	This function computes the distribution function for the
 *	gamma distribution with shape parameter alph and scale parameter
 *	scale.	This is also known as the incomplete gamma function.
 *	See Abramowitz and Stegun (6.5.1) for example.
 *
 *  NOTES
 *
 *	Complete redesign by Morten Welinder, originally for Gnumeric.
 *	Improvements (e.g. "while NEEDED_SCALE") by Martin Maechler
 *
 *  REFERENCES
 *
 */

#include "Rmath.h"
#include "dpq.h"
/*----------- DEBUGGING -------------
 * make CFLAGS='-DDEBUG_p -g'
 * (cd `R-devel RHOME`/src/nmath; gcc -I. -I../../src/include -I../../../R/src/include  -DHAVE_CONFIG_H -fopenmp -DDEBUG_p -g -c ../../../R/src/nmath/pgamma.c -o pgamma.o)
 */

/* Scalefactor:= (2^32)^8 = 2^256 = 1.157921e+77 */
#define SQR(x) ((x)*(x))
static const double scalefactor = SQR(SQR(SQR(4294967296.0)));
#undef SQR

/* If |x| > |k| * M_cutoff,  then  log[ exp(-x) * k^x ]	 =~=  -x */
static const double M_cutoff = M_LN2 * DBL_MAX_EXP / DBL_EPSILON;/*=3.196577e18*/

/* Continued fraction for calculation of
 *    1/i + x/(i+d) + x^2/(i+2*d) + x^3/(i+3*d) + ... = sum_{k=0}^Inf x^k/(i+k*d)
 *
 * auxilary in log1pmx() and lgamma1p()
 */
static double
logcf (double x, double i, double d,
       double eps /* ~ relative tolerance */)
{
    double c1 = 2 * d;
    double c2 = i + d;
    double c4 = c2 + d;
    double a1 = c2;
    double b1 = i * (c2 - i * x);
    double b2 = d * d * x;
    double a2 = c4 * c2 - b2;

#if 0
    assert (i > 0);
    assert (d >= 0);
#endif

    b2 = c4 * b1 - i * b2;

    while (fabs(a2 * b1 - a1 * b2) > fabs(eps * b1 * b2)) {
	double c3 = c2*c2*x;
	c2 += d;
	c4 += d;
	a1 = c4 * a2 - c3 * a1;
	b1 = c4 * b2 - c3 * b1;

	c3 = c1 * c1 * x;
	c1 += d;
	c4 += d;
	a2 = c4 * a1 - c3 * a2;
	b2 = c4 * b1 - c3 * b2;

	if (fabs (b2) > scalefactor) {
	    a1 /= scalefactor;
	    b1 /= scalefactor;
	    a2 /= scalefactor;
	    b2 /= scalefactor;
	} else if (fabs (b2) < 1 / scalefactor) {
	    a1 *= scalefactor;
	    b1 *= scalefactor;
	    a2 *= scalefactor;
	    b2 *= scalefactor;
	}
    }

    return a2 / b2;
}

/* Accurate calculation of log(1+x)-x, particularly for small x.  */
double log1pmx (double x)
{
    static const double minLog1Value = -0.79149064;

    if (x > 1 || x < minLog1Value)
	return log1p(x) - x;
    else { /* -.791 <=  x <= 1  -- expand in  [x/(2+x)]^2 =: y :
	    * log(1+x) - x =  x/(2+x) * [ 2 * y * S(y) - x],  with
	    * ---------------------------------------------
	    * S(y) = 1/3 + y/5 + y^2/7 + ... = \sum_{k=0}^\infty  y^k / (2k + 3)
	   */
	double r = x / (2 + x), y = r * r;
	if (fabs(x) < 1e-2) {
	    static const double two = 2;
	    return r * ((((two / 9 * y + two / 7) * y + two / 5) * y +
			    two / 3) * y - x);
	} else {
	    static const double tol_logcf = 1e-14;
	    return r * (2 * y * logcf (y, 3, 2, tol_logcf) - x);
	}
    }
}


/* Compute  log(gamma(a+1))  accurately also for small a (0 < a < 0.5). */
double lgamma1p (double a)
{
    const double eulers_const =	 0.5772156649015328606065120900824024;

    /* coeffs[i] holds (zeta(i+2)-1)/(i+2) , i = 0:(N-1), N = 40 : */
    const int N = 40;
    static const double coeffs[40] = {
	0.3224670334241132182362075833230126e-0,/* = (zeta(2)-1)/2 */
	0.6735230105319809513324605383715000e-1,/* = (zeta(3)-1)/3 */
	0.2058080842778454787900092413529198e-1,
	0.7385551028673985266273097291406834e-2,
	0.2890510330741523285752988298486755e-2,
	0.1192753911703260977113935692828109e-2,
	0.5096695247430424223356548135815582e-3,
	0.2231547584535793797614188036013401e-3,
	0.9945751278180853371459589003190170e-4,
	0.4492623673813314170020750240635786e-4,
	0.2050721277567069155316650397830591e-4,
	0.9439488275268395903987425104415055e-5,
	0.4374866789907487804181793223952411e-5,
	0.2039215753801366236781900709670839e-5,
	0.9551412130407419832857179772951265e-6,
	0.4492469198764566043294290331193655e-6,
	0.2120718480555466586923135901077628e-6,
	0.1004322482396809960872083050053344e-6,
	0.4769810169363980565760193417246730e-7,
	0.2271109460894316491031998116062124e-7,
	0.1083865921489695409107491757968159e-7,
	0.5183475041970046655121248647057669e-8,
	0.2483674543802478317185008663991718e-8,
	0.1192140140586091207442548202774640e-8,
	0.5731367241678862013330194857961011e-9,
	0.2759522885124233145178149692816341e-9,
	0.1330476437424448948149715720858008e-9,
	0.6422964563838100022082448087644648e-10,
	0.3104424774732227276239215783404066e-10,
	0.1502138408075414217093301048780668e-10,
	0.7275974480239079662504549924814047e-11,
	0.3527742476575915083615072228655483e-11,
	0.1711991790559617908601084114443031e-11,
	0.8315385841420284819798357793954418e-12,
	0.4042200525289440065536008957032895e-12,
	0.1966475631096616490411045679010286e-12,
	0.9573630387838555763782200936508615e-13,
	0.4664076026428374224576492565974577e-13,
	0.2273736960065972320633279596737272e-13,
	0.1109139947083452201658320007192334e-13/* = (zeta(40+1)-1)/(40+1) */
    };

    const double c = 0.2273736845824652515226821577978691e-12;/* zeta(N+2)-1 */
    const double tol_logcf = 1e-14;
    double lgam;
    int i;

    if (fabs (a) >= 0.5)
	return lgammafn (a + 1);

    /* Abramowitz & Stegun 6.1.33 : for |x| < 2,
     * <==> log(gamma(1+x)) = -(log(1+x) - x) - gamma*x + x^2 * \sum_{n=0}^\infty c_n (-x)^n
     * where c_n := (Zeta(n+2) - 1)/(n+2)  = coeffs[n]
     *
     * Here, another convergence acceleration trick is used to compute
     * lgam(x) :=  sum_{n=0..Inf} c_n (-x)^n
     */
    lgam = c * logcf(-a / 2, N + 2, 1, tol_logcf);
    for (i = N - 1; i >= 0; i--)
	lgam = coeffs[i] - a * lgam;

    return (a * lgam - eulers_const) * a - log1pmx (a);
} /* lgamma1p */



/*
 * Compute the log of a sum from logs of terms, i.e.,
 *
 *     log (exp (logx) + exp (logy))
 *
 * without causing overflows and without throwing away large handfuls
 * of accuracy.
 */
double logspace_add (double logx, double logy)
{
    return fmax2 (logx, logy) + log1p (exp (-fabs (logx - logy)));
}


/*
 * Compute the log of a difference from logs of terms, i.e.,
 *
 *     log (exp (logx) - exp (logy))
 *
 * without causing overflows and without throwing away large handfuls
 * of accuracy.
 */
double logspace_sub (double logx, double logy)
{
    return logx + R_Log1_Exp(logy - logx);
}

/*
 * Compute the log of a sum from logs of terms, i.e.,
 *
 *     log (sum_i  exp (logx[i]) ) =
 *     log (e^M * sum_i  e^(logx[i] - M) ) =
 *     M + log( sum_i  e^(logx[i] - M)
 *
 * without causing overflows or throwing much accuracy.
 */
#ifdef MATH_HAVE_LONG_DOUBLE
# define EXP expl
# define LOG logl
#else
# define EXP exp
# define LOG log
#endif
double logspace_sum (const double* logx, int n)
{
    if(n == 0) return ML_NEGINF; // = log( sum(<empty>) )
    if(n == 1) return logx[0];
    if(n == 2) return logspace_add(logx[0], logx[1]);
    // else (n >= 3) :
    int i;
    // Mx := max_i log(x_i)
    double Mx = logx[0];
    for(i = 1; i < n; i++) if(Mx < logx[i]) Mx = logx[i];
    LDOUBLE s = (LDOUBLE) 0.;
    for(i = 0; i < n; i++) s += EXP(logx[i] - Mx);
    return Mx + (double) LOG(s);
}



/* dpois_wrap (x__1, lambda) := dpois(x__1 - 1, lambda);  where
 * dpois(k, L) := exp(-L) L^k / gamma(k+1)  {the usual Poisson probabilities}
 *
 * and  dpois*(.., give_log = TRUE) :=  log( dpois*(..) )
*/
static double
dpois_wrap (double x_plus_1, double lambda, int give_log)
{
    if (!R_FINITE(lambda))
	return R_D__0;
    if (x_plus_1 > 1)
	return dpois_raw (x_plus_1 - 1, lambda, give_log);
    if (lambda > fabs(x_plus_1 - 1) * M_cutoff)
	return R_D_exp(-lambda - lgammafn(x_plus_1));
    else {
	double d = dpois_raw (x_plus_1, lambda, give_log);

	return give_log
	    ? d + log (x_plus_1 / lambda)
	    : d * (x_plus_1 / lambda);
    }
}

/*
 * Abramowitz and Stegun 6.5.29 [right]
 */
static double
pgamma_smallx (double x, double alph, int lower_tail, int log_p)
{
    double sum = 0, c = alph, n = 0, term;

    /*
     * Relative to 6.5.29 all terms have been multiplied by alph
     * and the first, thus being 1, is omitted.
     */

    do {
	n++;
	c *= -x / n;
	term = c / (alph + n);
	sum += term;
    } while (fabs (term) > DBL_EPSILON * fabs (sum));

    if (lower_tail) {
	double f1 = log_p ? log1p (sum) : 1 + sum;
	double f2;
	if (alph > 1) {
	    f2 = dpois_raw (alph, x, log_p);
	    f2 = log_p ? f2 + x : f2 * exp (x);
	} else if (log_p)
	    f2 = alph * log (x) - lgamma1p (alph);
	else
	    f2 = pow (x, alph) / exp (lgamma1p (alph));

	return log_p ? f1 + f2 : f1 * f2;
    } else {
	double lf2 = alph * log (x) - lgamma1p (alph);

	if (log_p)
	    return R_Log1_Exp (log1p (sum) + lf2);
	else {
	    double f1m1 = sum;
	    double f2m1 = expm1 (lf2);
	    return -(f1m1 + f2m1 + f1m1 * f2m1);
	}
    }
} /* pgamma_smallx() */

static double
pd_upper_series (double x, double y, int log_p)
{
    double term = x / y;
    double sum = term;

    do {
	y++;
	term *= x / y;
	sum += term;
    } while (term > sum * DBL_EPSILON);

    /* sum =  \sum_{n=1}^ oo  x^n     / (y*(y+1)*...*(y+n-1))
     *	   =  \sum_{n=0}^ oo  x^(n+1) / (y*(y+1)*...*(y+n))
     *	   =  x/y * (1 + \sum_{n=1}^oo	x^n / ((y+1)*...*(y+n)))
     *	   ~  x/y +  o(x/y)   {which happens when alph -> Inf}
     */
    return log_p ? log (sum) : sum;
}

/* Continued fraction for calculation of
 *    scaled upper-tail F_{gamma}
 *  ~=  (y / d) * [1 +  (1-y)/d +  O( ((1-y)/d)^2 ) ]
 */
static double
pd_lower_cf (double y, double d)
{
    double f= 0.0 /* -Wall */, of, f0;
    double i, c2, c3, c4,  a1, b1,  a2, b2;

#define	NEEDED_SCALE				\
	  (b2 > scalefactor) {			\
	    a1 /= scalefactor;			\
	    b1 /= scalefactor;			\
	    a2 /= scalefactor;			\
	    b2 /= scalefactor;			\
	}

#define max_it 200000

    if (y == 0) return 0;

    f0 = y/d;
    /* Needed, e.g. for  pgamma(10^c(100,295), shape= 1.1, log=TRUE): */
    if(fabs(y - 1) < fabs(d) * DBL_EPSILON) { /* includes y < d = Inf */

	return (f0);
    }

    if(f0 > 1.) f0 = 1.;
    c2 = y;
    c4 = d; /* original (y,d), *not* potentially scaled ones!*/

    a1 = 0; b1 = 1;
    a2 = y; b2 = d;

    while NEEDED_SCALE

    i = 0; of = -1.; /* far away */
    while (i < max_it) {

	i++;	c2--;	c3 = i * c2;	c4 += 2;
	/* c2 = y - i,  c3 = i(y - i),  c4 = d + 2i,  for i odd */
	a1 = c4 * a2 + c3 * a1;
	b1 = c4 * b2 + c3 * b1;

	i++;	c2--;	c3 = i * c2;	c4 += 2;
	/* c2 = y - i,  c3 = i(y - i),  c4 = d + 2i,  for i even */
	a2 = c4 * a1 + c3 * a2;
	b2 = c4 * b1 + c3 * b2;

	if NEEDED_SCALE

	if (b2 != 0) {
	    f = a2 / b2;
	    /* convergence check: relative; "absolute" for very small f : */
	    if (fabs (f - of) <= DBL_EPSILON * fmax2(f0, fabs(f))) {
		return f;
	    }
	    of = f;
	}
    }

   // MATHLIB_WARNING(" ** NON-convergence in pgamma()'s pd_lower_cf() f= %g.\n", f);
    return f;/* should not happen ... */
} /* pd_lower_cf() */
#undef NEEDED_SCALE


static double
pd_lower_series (double lambda, double y)
{
    double term = 1, sum = 0;

    while (y >= 1 && term > sum * DBL_EPSILON) {
	term *= y / lambda;
	sum += term;
	y--;
    }
    /* sum =  \sum_{n=0}^ oo  y*(y-1)*...*(y - n) / lambda^(n+1)
     *	   =  y/lambda * (1 + \sum_{n=1}^Inf  (y-1)*...*(y-n) / lambda^n)
     *	   ~  y/lambda + o(y/lambda)
     */

    if (y != floor (y)) {
	/*
	 * The series does not converge as the terms start getting
	 * bigger (besides flipping sign) for y < -lambda.
	 */
	double f;

	/* FIXME: in quite few cases, adding  term*f  has no effect (f too small)
	 *	  and is unnecessary e.g. for pgamma(4e12, 121.1) */
	f = pd_lower_cf (y, lambda + 1 - y);

	sum += term * f;
    }

    return sum;
} /* pd_lower_series() */

/*
 * Compute the following ratio with higher accuracy that would be had
 * from doing it directly.
 *
 *		 dnorm (x, 0, 1, FALSE)
 *	   ----------------------------------
 *	   pnorm (x, 0, 1, lower_tail, FALSE)
 *
 * Abramowitz & Stegun 26.2.12
 */
static double
dpnorm (double x, int lower_tail, double lp)
{
    /*
     * So as not to repeat a pnorm call, we expect
     *
     *	 lp == pnorm (x, 0, 1, lower_tail, TRUE)
     *
     * but use it only in the non-critical case where either x is small
     * or p==exp(lp) is close to 1.
     */

    if (x < 0) {
	x = -x;
	lower_tail = !lower_tail;
    }

    if (x > 10 && !lower_tail) {
	double term = 1 / x;
	double sum = term;
	double x2 = x * x;
	double i = 1;

	do {
	    term *= -i / x2;
	    sum += term;
	    i += 2;
	} while (fabs (term) > DBL_EPSILON * sum);

	return 1 / sum;
    } else {
	double d = dnorm (x, 0., 1., 0);
	return d / exp (lp);
    }
}

/*
 * Asymptotic expansion to calculate the probability that Poisson variate
 * has value <= x.
 * Various assertions about this are made (without proof) at
 * http://members.aol.com/iandjmsmith/PoissonApprox.htm
 */
static double
ppois_asymp (double x, double lambda, int lower_tail, int log_p)
{
    static const double coefs_a[8] = {
	-1e99, /* placeholder used for 1-indexing */
	2/3.,
	-4/135.,
	8/2835.,
	16/8505.,
	-8992/12629925.,
	-334144/492567075.,
	698752/1477701225.
    };

    static const double coefs_b[8] = {
	-1e99, /* placeholder */
	1/12.,
	1/288.,
	-139/51840.,
	-571/2488320.,
	163879/209018880.,
	5246819/75246796800.,
	-534703531/902961561600.
    };

    double elfb, elfb_term;
    double res12, res1_term, res1_ig, res2_term, res2_ig;
    double dfm, pt_, s2pt, f, np;
    int i;

    dfm = lambda - x;
    /* If lambda is large, the distribution is highly concentrated
       about lambda.  So representation error in x or lambda can lead
       to arbitrarily large values of pt_ and hence divergence of the
       coefficients of this approximation.
    */
    pt_ = - log1pmx (dfm / x);
    s2pt = sqrt (2 * x * pt_);
    if (dfm < 0) s2pt = -s2pt;

    res12 = 0;
    res1_ig = res1_term = sqrt (x);
    res2_ig = res2_term = s2pt;
    for (i = 1; i < 8; i++) {
	res12 += res1_ig * coefs_a[i];
	res12 += res2_ig * coefs_b[i];
	res1_term *= pt_ / i ;
	res2_term *= 2 * pt_ / (2 * i + 1);
	res1_ig = res1_ig / x + res1_term;
	res2_ig = res2_ig / x + res2_term;
    }

    elfb = x;
    elfb_term = 1;
    for (i = 1; i < 8; i++) {
	elfb += elfb_term * coefs_b[i];
	elfb_term /= x;
    }
    if (!lower_tail) elfb = -elfb;

    f = res12 / elfb;

    np = pnorm (s2pt, 0.0, 1.0, !lower_tail, log_p);

    if (log_p) {
	double n_d_over_p = dpnorm (s2pt, !lower_tail, np);

	return np + log1p (f * n_d_over_p);
    } else {
	double nd = dnorm (s2pt, 0., 1., log_p);

	return np + f * nd;
    }
} /* ppois_asymp() */


double pgamma_raw (double x, double alph, int lower_tail, int log_p)
{
/* Here, assume that  (x,alph) are not NA  &  alph > 0 . */

    double res;

    R_P_bounds_01(x, 0., ML_POSINF);

    if (x < 1) {
	res = pgamma_smallx (x, alph, lower_tail, log_p);
    } else if (x <= alph - 1 && x < 0.8 * (alph + 50)) {
	/* incl. large alph compared to x */
	double sum = pd_upper_series (x, alph, log_p);/* = x/alph + o(x/alph) */
	double d = dpois_wrap (alph, x, log_p);

	if (!lower_tail)
	    res = log_p
		? R_Log1_Exp (d + sum)
		: 1 - d * sum;
	else
	    res = log_p ? sum + d : sum * d;
    } else if (alph - 1 < x && alph < 0.8 * (x + 50)) {
	/* incl. large x compared to alph */
	double sum;
	double d = dpois_wrap (alph, x, log_p);

	if (alph < 1) {
	    if (x * DBL_EPSILON > 1 - alph)
		sum = R_D__1;
	    else {
		double f = pd_lower_cf (alph, x - (alph - 1)) * x / alph;
		/* = [alph/(x - alph+1) + o(alph/(x-alph+1))] * x/alph = 1 + o(1) */
		sum = log_p ? log (f) : f;
	    }
	} else {
	    sum = pd_lower_series (x, alph - 1);/* = (alph-1)/x + o((alph-1)/x) */
	    sum = log_p ? log1p (sum) : 1 + sum;
	}

	if (!lower_tail)
	    res = log_p ? sum + d : sum * d;
	else
	    res = log_p
		? R_Log1_Exp (d + sum)
		: 1 - d * sum;
    } else { /* x >= 1 and x fairly near alph. */

	res = ppois_asymp (alph - 1, x, !lower_tail, log_p);
    }

    /*
     * We lose a fair amount of accuracy to underflow in the cases
     * where the final result is very close to DBL_MIN.	 In those
     * cases, simply redo via log space.
     */
    if (!log_p && res < DBL_MIN / DBL_EPSILON) {
	/* with(.Machine, double.xmin / double.eps) #|-> 1.002084e-292 */

	return exp (pgamma_raw (x, alph, lower_tail, 1));
    } else
	return res;
}


double pgamma(double x, double alph, double scale, int lower_tail, int log_p)
{
#ifdef IEEE_754
    if (ISNAN(x) || ISNAN(alph) || ISNAN(scale))
	return x + alph + scale;
#endif
    if(alph < 0. || scale <= 0.)
	return ML_NAN;
    x /= scale;
#ifdef IEEE_754
    if (ISNAN(x)) /* eg. original x = scale = +Inf */
	return x;
#endif
    if(alph == 0.) /* limit case; useful e.g. in pnchisq() */
	return (x <= 0) ? R_DT_0: R_DT_1; /* <= assert  pgamma(0,0) ==> 0 */
    return pgamma_raw (x, alph, lower_tail, log_p);
}
/* From: terra@gnome.org (Morten Welinder)
 * To: R-bugs@biostat.ku.dk
 * Cc: maechler@stat.math.ethz.ch
 * Subject: Re: [Rd] pgamma discontinuity (PR#7307)
 * Date: Tue, 11 Jan 2005 13:57:26 -0500 (EST)

 * this version of pgamma appears to be quite good and certainly a vast
 * improvement over current R code.  (I last looked at 2.0.1)  Apart from
 * type naming, this is what I have been using for Gnumeric 1.4.1.

 * This could be included into R as-is, but you might want to benefit from
 * making logcf, log1pmx, lgamma1p, and possibly logspace_add/logspace_sub
 * available to other parts of R.

 * MM: I've not (yet?) taken  logcf(), but the other four
 */
