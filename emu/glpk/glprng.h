/* glprng.h (pseudo-random number generator) */

/*----------------------------------------------------------------------
-- This code is part of GNU Linear Programming Kit (GLPK).
--
-- Copyright (C) 2000, 01, 02, 03, 04, 05, 06, 07 Andrew Makhorin,
-- Department for Applied Informatics, Moscow Aviation Institute,
-- Moscow, Russia. All rights reserved. E-mail: <mao@mai2.rcnet.ru>.
--
-- GLPK is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2, or (at your option)
-- any later version.
--
-- GLPK is distributed in the hope that it will be useful, but WITHOUT
-- ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
-- or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
-- License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with GLPK; see the file COPYING. If not, write to the Free
-- Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
-- 02110-1301, USA.
----------------------------------------------------------------------*/

#ifndef _GLPRNG_H
#define _GLPRNG_H

typedef struct RNG RNG;

struct RNG
{     /* Knuth's portable pseudo-random number generator */
      int A[56];
      /* pseudo-random values */
      int *fptr;
      /* the next A value to be exported */
};

#define rng_create_rand       _glp_rng_create_rand
#define rng_init_rand         _glp_rng_init_rand
#define rng_next_rand         _glp_rng_next_rand
#define rng_unif_rand         _glp_rng_unif_rand
#define rng_delete_rand       _glp_rng_delete_rand

RNG *rng_create_rand(void);
/* create pseudo-random number generator */

void rng_init_rand(RNG *rand, int seed);
/* initialize pseudo-random number generator */

int rng_next_rand(RNG *rand);
/* obtain pseudo-random integer in [0, 2^31-1] */

int rng_unif_rand(RNG *rand, int m);
/* obtain pseudo-random integer in [0, m-1] */

void rng_delete_rand(RNG *rand);
/* delete pseudo-random number generator */

#endif

/* eof */
