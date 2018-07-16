/* glpbfi.c */

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

typedef struct BFI BFI;
#define _GLPBFI_DEFINED
#include "glpbfi.h"
#include "glpinv.h"
#include "glplib.h"

struct BFI
{     int m_max;
      int valid;
      INV *inv;
};

BFI *bfi_create_binv(void)
{     /* create factorization of the basis matrix */
      BFI *binv;
      binv = xmalloc(sizeof(BFI));
      binv->m_max = 0;
      binv->valid = 0;
      binv->inv = NULL;
      return binv;
}

int bfi_is_valid(BFI *binv)
{     /* check if factorization is valid */
      return binv->valid;
}

void bfi_invalidate(BFI *binv)
{     /* invalidate factorization of the basis matrix */
      binv->valid = 0;
      return;
}

int bfi_factorize(BFI *binv, int m,
      void *info, int (*col)(void *info, int j, int rn[], double bj[]))
{     /* compute factorization of the basis matrix */
      int ret;
      xassert(m > 0);
      if (binv->m_max < m)
      {  if (binv->inv != NULL) inv_delete(binv->inv);
         binv->m_max = m + 100;
         binv->inv = inv_create(binv->m_max, 50);
      }
      binv->inv->m = m;
      binv->inv->luf->n = m;
      ret = inv_decomp(binv->inv, info, col);
      binv->valid = (ret == 0);
      return ret;
}

int bfi_factorize1(BFI *binv, int m,
      void *info, int (*col)(void *info, int j, int rn[], double bj[]),
      int *def, int bad[], int good[])
{     /* compute factorization of the basis matrix */
      int j, ret;
      xassert(m > 0);
      if (binv->m_max < m)
      {  if (binv->inv != NULL) inv_delete(binv->inv);
         binv->m_max = m + 100;
         binv->inv = inv_create(binv->m_max, 50);
      }
      binv->inv->m = m;
      binv->inv->luf->n = m;
      ret = inv_decomp(binv->inv, info, col);
      binv->valid = (ret == 0);
      *def = 0;
      for (j = binv->inv->luf->rank+1; j <= m; j++)
      {  (*def)++;
         bad[*def] = binv->inv->luf->qq_col[j];
         good[*def] = binv->inv->luf->pp_row[j];
      }
      return ret;
}

void bfi_ftran_dense(BFI *binv, double x[], int save)
{     /* perform FTRAN (dense version) */
      xassert(binv->valid);
      inv_ftran(binv->inv, x, save);
      return;
}

int bfi_ftran_sparse(BFI *binv, int ne, int ind[], double vec[],
      int save)
{     /* perform FTRAN (sparse version) */
      xassert(binv->valid);
      ne = inv_ftran_sparse(binv->inv, ne, ind, vec, save);
      return ne;
}

void bfi_btran_dense(BFI *binv, double x[])
{     /* perform BTRAN (dense version) */
      xassert(binv->valid);
      inv_btran(binv->inv, x);
      return;
}

int bfi_btran_sparse(BFI *binv, int ne, int ind[], double vec[])
{     /* perform BTRAN (sparse version) */
      xassert(binv->valid);
      ne = inv_btran_sparse(binv->inv, ne, ind, vec);
      return ne;
}

int bfi_update_binv(BFI *binv, int j)
{     /* update factorization of the basis matrix */
      int ret;
      xassert(binv->valid);
      ret = inv_update(binv->inv, j);
      binv->valid = (ret == 0);
      return ret;
}

void bfi_delete_binv(BFI *binv)
{     /* delete factorization of the basis matrix */
      if (binv->inv != NULL) inv_delete(binv->inv);
      xfree(binv);
      return;
}

/* eof */
