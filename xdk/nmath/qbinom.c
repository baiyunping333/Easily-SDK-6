﻿/*
 *  Mathlib : A C Library of Special Functions
 *  Copyright (C) 1998 Ross Ihaka
 *  Copyright (C) 2000-2009 The R Core Team
 *  Copyright (C) 2003-2009 The R Foundation
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
 *  DESCRIPTION
 *
 *	The quantile function of the binomial distribution.
 *
 *  METHOD
 *
 *	Uses the Cornish-Fisher Expansion to include a skewness
 *	correction to a normal approximation.  This gives an
 *	initial value which never seems to be off by more than
 *	1 or 2.	 A search is then conducted of values close to
 *	this initial start point.
 */
#include "Rmath.h"
#include "dpq.h"

static double
do_search(double y, double *z, double p, double n, double pr, double incr)
{
    if(*z >= p) {
			/* search to the left */

	for(;;) {
	    double newz;
	    if(y == 0 ||
	       (newz = pbinom(y - incr, n, pr, /*l._t.*/1, /*log_p*/0)) < p)
		return y;
	    y = fmax2(0, y - incr);
	    *z = newz;
	}
    }
    else {		/* search to the right */

	for(;;) {
	    y = fmin2(y + incr, n);
	    if(y == n ||
	       (*z = pbinom(y, n, pr, /*l._t.*/1, /*log_p*/0)) >= p)
		return y;
	}
    }
}


double qbinom(double p, double n, double pr, int lower_tail, int log_p)
{
    double q, mu, sigma, gamma, z, y;

#ifdef IEEE_754
    if (ISNAN(p) || ISNAN(n) || ISNAN(pr))
	return p + n + pr;
#endif
    if(!R_FINITE(n) || !R_FINITE(pr))
	return ML_NAN;
    /* if log_p is true, p = -Inf is a legitimate value */
    if(!R_FINITE(p) && !log_p)
	return ML_NAN;

    if(n != floor(n + 0.5)) return ML_NAN;
    if (pr < 0 || pr > 1 || n < 0)
	return ML_NAN;

    R_Q_P01_boundaries(p, 0, n);

    if (pr == 0. || n == 0) return 0.;

    q = 1 - pr;
    if(q == 0.) return n; /* covers the full range of the distribution */
    mu = n * pr;
    sigma = sqrt(n * pr * q);
    gamma = (q - pr) / sigma;

    /* Note : "same" code in qpois.c, qbinom.c, qnbinom.c --
     * FIXME: This is far from optimal [cancellation for p ~= 1, etc]: */
    if(!lower_tail || log_p) {
	p = R_DT_qIv(p); /* need check again (cancellation!): */
	if (p == 0.) return 0.;
	if (p == 1.) return n;
    }
    /* temporary hack --- FIXME --- */
    if (p + 1.01*DBL_EPSILON >= 1.) return n;

    /* y := approx.value (Cornish-Fisher expansion) :  */
    z = qnorm(p, 0., 1., /*lower_tail*/1, /*log_p*/0);
    y = floor(mu + sigma * (z + gamma * (z*z - 1) / 6) + 0.5);

    if(y > n) /* way off */ y = n;

    z = pbinom(y, n, pr, /*lower_tail*/1, /*log_p*/0);

    /* fuzz to ensure left continuity: */
    p *= 1 - 64*DBL_EPSILON;

    if(n < 1e5) return do_search(y, &z, p, n, pr, 1);
    /* Otherwise be a bit cleverer in the search */
    {
	double incr = floor(n * 0.001), oldincr;
	do {
	    oldincr = incr;
	    y = do_search(y, &z, p, n, pr, incr);
	    incr = fmax2(1, floor(incr/100));
	} while(oldincr > 1 && incr > n*1e-15);
	return y;
    }
}
