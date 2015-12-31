/* glpbfi.h (basis factorization interface) */

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

#ifndef _GLPBFI_H
#define _GLPBFI_H

#ifndef _GLPBFI_DEFINED
#define _GLPBFI_DEFINED
typedef struct { int none_; } BFI;
#endif

#define bfi_create_binv       _glp_bfi_create_binv
#define bfi_is_valid          _glp_bfi_is_valid
#define bfi_invalidate        _glp_bfi_invalidate
#define bfi_factorize         _glp_bfi_factorize
#define bfi_factorize1        _glp_bfi_factorize1
#define bfi_ftran_dense       _glp_bfi_ftran_dense
#define bfi_ftran_sparse      _glp_bfi_ftran_sparse
#define bfi_btran_dense       _glp_bfi_btran_dense
#define bfi_btran_sparse      _glp_bfi_btran_sparse
#define bfi_update_binv       _glp_bfi_update_binv
#define bfi_delete_binv       _glp_bfi_delete_binv

BFI *bfi_create_binv(void);
/* create factorization of the basis matrix */

int bfi_is_valid(BFI *binv);
/* check if factorization is valid */

void bfi_invalidate(BFI *binv);
/* invalidate factorization of the basis matrix */

int bfi_factorize(BFI *binv, int m,
      void *info, int (*col)(void *info, int j, int rn[], double bj[]));
/* compute factorization of the basis matrix */

int bfi_factorize1(BFI *binv, int m,
      void *info, int (*col)(void *info, int j, int rn[], double bj[]),
      int *def, int bad[], int good[]);
/* compute factorization of the basis matrix */

void bfi_ftran_dense(BFI *binv, double x[], int save);
/* perform FTRAN (dense version) */

int bfi_ftran_sparse(BFI *binv, int ne, int ind[], double vec[],
      int save);
/* perform FTRAN (sparse version) */

void bfi_btran_dense(BFI *binv, double x[]);
/* perform BTRAN (dense version) */

int bfi_btran_sparse(BFI *binv, int ne, int ind[], double vec[]);
/* perform BTRAN (sparse version) */

int bfi_update_binv(BFI *binv, int j);
/* update factorization of the basis matrix */

void bfi_delete_binv(BFI *binv);
/* delete factorization of the basis matrix */

#endif

/* eof */
