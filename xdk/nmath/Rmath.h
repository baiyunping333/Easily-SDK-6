/* -*- C -*-
 *  Mathlib : A C Library of Special Functions
 *  Copyright (C) 1998-2016  The R Core Team
 *  Copyright (C) 2004       The R Foundation
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, a copy is available at
 *  https://www.R-project.org/Licenses/
 *

 * Rmath.h  should contain ALL headers from R's C code in `src/nmath'
   -------  such that ``the Math library'' can be used by simply

   ``#include <Rmath.h> ''

   and nothing else.

   It is part of the API and supports 'standalone Rmath'.

*/
#ifndef RMATH_H
#define RMATH_H

#include "Rarith.h"


/* Used internally only */
double  Rf_d1mach(int);
double	Rf_gamma_cody(double);

/* R's versions with !R_FINITE checks */

double R_pow(double x, double y);
double R_pow_di(double, int);

# define R_forceint(x)   round(x)
//R >= 3.1.0: # define R_nonint(x) 	  (fabs((x) - R_forceint(x)) > 1e-7)
# define R_nonint(x) 	  (fabs((x) - R_forceint(x)) > 1e-7*fmax2(1., fabs(x)))

int R_IsNA(double);		/* True for R's NA only */
int R_IsNaN(double);		/* True for special NaN, *not* for NA */
int R_finite(double);		/* True if none of NA, NaN, +/-Inf */

/* The following is only defined inside R */
#ifdef MATH_HAVE_ISFINITE
/* isfinite is defined in <math.h> according to C99 */
# define R_FINITE(x)    isfinite(x)
#else
# define R_FINITE(x)    R_finite(x)
#endif



/* Chebyshev Series */

int	chebyshev_init(double*, int, double);
double	chebyshev_eval(double, const double *, const int);

/* Gamma and Related Functions */

void	gammalims(double*, double*);
double	lgammacor(double); /* log(gamma) correction */
double  stirlerr(double);  /* Stirling expansion "error" */

double	lfastchoose(double, double);

double  bd0(double, double);

double  pnchisq_raw(double, double, double, double, double,
	int, int, int);
double  pgamma_raw(double, double, int, int);
double	pbeta_raw(double, double, double, int, int);
double  qchisq_appr(double, double, double, int, int, double tol);
LDOUBLE pnbeta_raw(double, double, double, double, double);
double	pnbeta2(double, double, double, double, double, int, int);

int	Rf_i1mach(int);

/* From toms708.c */
void bratio(double a, double b, double x, double y,
	double *w, double *w1, int *ierr, int log_p);

double currentTime(void);
unsigned int TimeToSeed(void);

double R_unif_index(double);


#ifdef  __cplusplus
extern "C" {
#endif

	/* Random Number Generators */

	EXP_API double	norm_rand(void);
	EXP_API double	unif_rand(void);
	EXP_API double	exp_rand(void);
#ifdef MATHLIB_STANDALONE
	EXP_API void	set_seed(unsigned int, unsigned int);
	EXP_API void	get_seed(unsigned int *, unsigned int *);
#endif
	/* Normal Distribution */

	EXP_API double	dnorm(double, double, double, int);
	EXP_API double	pnorm(double, double, double, int, int);
	EXP_API double	qnorm(double, double, double, int, int);
	EXP_API double	rnorm(double, double);
	EXP_API void	pnorm_both(double, double *, double *, int, int);/* both tails */

	/* Uniform Distribution */

	EXP_API double	dunif(double, double, double, int);
	EXP_API double	punif(double, double, double, int, int);
	EXP_API double	qunif(double, double, double, int, int);
	EXP_API double	runif(double, double);

	/* Gamma Distribution */

	EXP_API double	dgamma(double, double, double, int);
	EXP_API double	pgamma(double, double, double, int, int);
	EXP_API double	qgamma(double, double, double, int, int);
	EXP_API double	rgamma(double, double);

	EXP_API double  log1pmx(double);
	EXP_API double  log1pexp(double); // <-- ../nmath/plogis.c
	EXP_API double  lgamma1p(double);
	EXP_API double  logspace_add(double, double);
	EXP_API double  logspace_sub(double, double);
	EXP_API double  logspace_sum(const double *, int);

	/* Beta Distribution */

	EXP_API double	dbeta(double, double, double, int);
	EXP_API double	pbeta(double, double, double, int, int);
	EXP_API double	qbeta(double, double, double, int, int);
	EXP_API double	rbeta(double, double);

	/* Lognormal Distribution */

	EXP_API double	dlnorm(double, double, double, int);
	EXP_API double	plnorm(double, double, double, int, int);
	EXP_API double	qlnorm(double, double, double, int, int);
	EXP_API double	rlnorm(double, double);

	/* Chi-squared Distribution */

	EXP_API double	dchisq(double, double, int);
	EXP_API double	pchisq(double, double, int, int);
	EXP_API double	qchisq(double, double, int, int);
	EXP_API double	rchisq(double);

	/* Non-central Chi-squared Distribution */

	EXP_API double	dnchisq(double, double, double, int);
	EXP_API double	pnchisq(double, double, double, int, int);
	EXP_API double	qnchisq(double, double, double, int, int);
	EXP_API double	rnchisq(double, double);

	/* F Distibution */

	EXP_API double	df(double, double, double, int);
	EXP_API double	pf(double, double, double, int, int);
	EXP_API double	qf(double, double, double, int, int);
	EXP_API double	rf(double, double);

	/* Student t Distibution */

	EXP_API double	dt(double, double, int);
	EXP_API double	pt(double, double, int, int);
	EXP_API double	qt(double, double, int, int);
	EXP_API double	rt(double);

	/* Binomial Distribution */

	EXP_API double  dbinom_raw(double x, double n, double p, double q, int give_log);
	EXP_API double	dbinom(double, double, double, int);
	EXP_API double	pbinom(double, double, double, int, int);
	EXP_API double	qbinom(double, double, double, int, int);
	EXP_API double	rbinom(double, double);

	/* Multnomial Distribution */

	EXP_API void	rmultinom(int, double*, int, int*);

	/* Cauchy Distribution */

	EXP_API double	dcauchy(double, double, double, int);
	EXP_API double	pcauchy(double, double, double, int, int);
	EXP_API double	qcauchy(double, double, double, int, int);
	EXP_API double	rcauchy(double, double);

	/* Exponential Distribution */

	EXP_API double	dexp(double, double, int);
	EXP_API double	pexp(double, double, int, int);
	EXP_API double	qexp(double, double, int, int);
	EXP_API double	rexp(double);

	/* Geometric Distribution */

	EXP_API double	dgeom(double, double, int);
	EXP_API double	pgeom(double, double, int, int);
	EXP_API double	qgeom(double, double, int, int);
	EXP_API double	rgeom(double);

	/* Hypergeometric Distibution */

	EXP_API double	dhyper(double, double, double, double, int);
	EXP_API double	phyper(double, double, double, double, int, int);
	EXP_API double	qhyper(double, double, double, double, int, int);
	EXP_API double	rhyper(double, double, double);

	/* Negative Binomial Distribution */

	EXP_API double	dnbinom(double, double, double, int);
	EXP_API double	pnbinom(double, double, double, int, int);
	EXP_API double	qnbinom(double, double, double, int, int);
	EXP_API double	rnbinom(double, double);

	EXP_API double	dnbinom_mu(double, double, double, int);
	EXP_API double	pnbinom_mu(double, double, double, int, int);
	EXP_API double	qnbinom_mu(double, double, double, int, int);
	EXP_API double	rnbinom_mu(double, double);

	/* Poisson Distribution */

	EXP_API double	dpois_raw(double, double, int);
	EXP_API double	dpois(double, double, int);
	EXP_API double	ppois(double, double, int, int);
	EXP_API double	qpois(double, double, int, int);
	EXP_API double	rpois(double);

	/* Weibull Distribution */

	EXP_API double	dweibull(double, double, double, int);
	EXP_API double	pweibull(double, double, double, int, int);
	EXP_API double	qweibull(double, double, double, int, int);
	EXP_API double	rweibull(double, double);

	/* Logistic Distribution */

	EXP_API double	dlogis(double, double, double, int);
	EXP_API double	plogis(double, double, double, int, int);
	EXP_API double	qlogis(double, double, double, int, int);
	EXP_API double	rlogis(double, double);

	/* Non-central Beta Distribution */

	EXP_API double	dnbeta(double, double, double, double, int);
	EXP_API double	pnbeta(double, double, double, double, int, int);
	EXP_API double	qnbeta(double, double, double, double, int, int);
	EXP_API double	rnbeta(double, double, double);

	/* Non-central F Distribution */

	EXP_API double  dnf(double, double, double, double, int);
	EXP_API double	pnf(double, double, double, double, int, int);
	EXP_API double	qnf(double, double, double, double, int, int);

	/* Non-central Student t Distribution */

	EXP_API double	dnt(double, double, double, int);
	EXP_API double	pnt(double, double, double, int, int);
	EXP_API double	qnt(double, double, double, int, int);

	/* Studentized Range Distribution */

	EXP_API double	ptukey(double, double, double, double, int, int);
	EXP_API double	qtukey(double, double, double, double, int, int);

	/* Wilcoxon Rank Sum Distribution */

	EXP_API double dwilcox(double, double, double, int);
	EXP_API double pwilcox(double, double, double, int, int);
	EXP_API double qwilcox(double, double, double, int, int);
	EXP_API double rwilcox(double, double);

	/* Wilcoxon Signed Rank Distribution */

	EXP_API double dsignrank(double, double, int);
	EXP_API double psignrank(double, double, int, int);
	EXP_API double qsignrank(double, double, int, int);
	EXP_API double rsignrank(double);

	/* Gamma and Related Functions */
	EXP_API double	gammafn(double);
	EXP_API double	lgammafn(double);
	EXP_API double	lgammafn_sign(double, int*);
	EXP_API void    dpsifn(double, int, int, int, double*, int*, int*);
	EXP_API double	psigamma(double, double);
	EXP_API double	digamma(double);
	EXP_API double	trigamma(double);
	EXP_API double	tetragamma(double);
	EXP_API double	pentagamma(double);

	EXP_API double	beta(double, double);
	EXP_API double	lbeta(double, double);

	EXP_API double	choose(double, double);
	EXP_API double	lchoose(double, double);

	/* Bessel Functions */

	EXP_API double	bessel_i(double, double, double);
	EXP_API double	bessel_j(double, double);
	EXP_API double	bessel_k(double, double, double);
	EXP_API double	bessel_y(double, double);
	EXP_API double	bessel_i_ex(double, double, double, double *);
	EXP_API double	bessel_j_ex(double, double, double *);
	EXP_API double	bessel_k_ex(double, double, double, double *);
	EXP_API double	bessel_y_ex(double, double, double *);


	/* General Support Functions */

#ifndef MATH_HAVE_HYPOT
	EXP_API double 	hypot(double, double);
#endif
	EXP_API double 	pythag(double, double);
#ifndef MATH_HAVE_EXPM1
	EXP_API double  expm1(double); /* = exp(x)-1 {care for small x} */
#endif
#ifndef MATH_HAVE_LOG1P
	EXP_API double  log1p(double); /* = log(1+x) {care for small x} */
#endif
	EXP_API int	imax2(int, int);
	EXP_API int	imin2(int, int);
	EXP_API double	fmax2(double, double);
	EXP_API double	fmin2(double, double);
	EXP_API double	sign(double);
	EXP_API double	fprec(double, double);
	EXP_API double	fround(double, double);
	EXP_API double	fsign(double, double);
	EXP_API double	ftrunc(double);

	EXP_API double  log1pmx(double); /* Accurate log(1+x) - x, {care for small x} */
	EXP_API double  lgamma1p(double);/* accurate log(gamma(x+1)), small x (0 < x < 0.5) */

/* More accurate cos(pi*x), sin(pi*x), tan(pi*x)

   These declarations might clash with system headers if someone had
   already included math.h with __STDC_WANT_IEC_60559_FUNCS_EXT__
   defined (and we try, above).
   We can add a check for that via the value of
   __STDC_IEC_60559_FUNCS__ (>= 201506L).
*/
	EXP_API double cospi(double);
	EXP_API double sinpi(double);
	EXP_API double tanpi(double);

/* Compute the log of a sum or difference from logs of terms, i.e.,
 *
 *     log (exp (logx) + exp (logy))
 * or  log (exp (logx) - exp (logy))
 *
 * without causing overflows or throwing away too much accuracy:
 */
	EXP_API double  logspace_add(double logx, double logy);
	EXP_API double  logspace_sub(double logx, double logy);



#ifdef  __cplusplus
}
#endif

#endif /* RMATH_H */
