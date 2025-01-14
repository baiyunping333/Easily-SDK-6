﻿/*
 *  Mathlib : A C Library of Special Functions
 *  Copyright (C) 1998 Ross Ihaka
 *  Copyright (C) 2000 The R Core Team
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
 *    Random variates from the Weibull distribution.
 */

#include "Rmath.h"

double rweibull(double shape, double scale)
{
    if (!R_FINITE(shape) || !R_FINITE(scale) || shape <= 0. || scale <= 0.) {
	if(scale == 0.) return 0.;
	/* else */
	return ML_NAN;
    }

    return scale * pow(-log(unif_rand()), 1.0 / shape);
}
