/* glpinv02.c */

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

#include "glpinv.h"
#include "glplib.h"
#define fault xfault1

int inv_h_sparse(INV *inv, int ne, int ind[], double vec[])
{     /* solve system H*x = b (pseudo-sparse version) */
      int m = inv->m;
      int k;
      inv_h_solve(inv, 0, vec);
      for (ne = 0, k = 1; k <= m; k++)
         if (vec[k] != 0.0) ind[++ne] = k;
      return ne;
}

int inv_ht_sparse(INV *inv, int ne, int ind[], double vec[])
{     /* solve system H'*x = b (pseudo-sparse version) */
      int m = inv->m;
      int k;
      inv_h_solve(inv, 1, vec);
      for (ne = 0, k = 1; k <= m; k++)
         if (vec[k] != 0.0) ind[++ne] = k;
      return ne;
}

int inv_ftran_sparse(INV *inv, int ne, int ind[], double vec[],
      int save)
{     /* perform FTRAN (sparse version) */
      int *pp_row = inv->luf->pp_row;
      int *pp_col = inv->luf->pp_col;
      int *p0_row = inv->p0_row;
      int *p0_col = inv->p0_col;
      double *work = inv->luf->work;
      int k, t;
      if (!inv->valid)
         fault("inv_ftran_sparse: the factorization is not valid");
      /* B = F*H*V, therefore inv(B) = inv(V)*inv(H)*inv(F) */
      for (t = 1; t <= ne; t++)
         k = ind[t], work[t] = vec[k], vec[k] = 0.0;
      inv->luf->pp_row = p0_row;
      inv->luf->pp_col = p0_col;
      ne = luf_f_sparse(inv->luf, ne, ind, work);
      inv->luf->pp_row = pp_row;
      inv->luf->pp_col = pp_col;
      for (t = 1; t <= ne; t++) vec[ind[t]] = work[t];
      ne = inv_h_sparse(inv, ne, ind, vec);
      for (t = 1; t <= ne; t++)
         k = ind[t], work[t] = vec[k], vec[k] = 0.0;
      /* save partially transformed column, if required */
      if (save)
      {  inv->cc_len = ne;
         memcpy(&inv->cc_ndx[1], &ind[1], ne * sizeof(int));
         memcpy(&inv->cc_val[1], &work[1], ne * sizeof(double));
      }
      ne = luf_v_sparse(inv->luf, ne, ind, work);
      for (t = 1; t <= ne; t++) vec[ind[t]] = work[t];
      return ne;
}

int inv_btran_sparse(INV *inv, int ne, int ind[], double vec[])
{     /* perform BTRAN (sparse version) */
      int *pp_row = inv->luf->pp_row;
      int *pp_col = inv->luf->pp_col;
      int *p0_row = inv->p0_row;
      int *p0_col = inv->p0_col;
      double *work = inv->luf->work;
      int k, t;
      if (!inv->valid)
         fault("inv_btran_sparse: the factorization is not valid");
      /* B = F*H*V, therefore inv(B') = inv(F')*inv(H')*inv(V') */
      for (t = 1; t <= ne; t++)
         k = ind[t], work[t] = vec[k], vec[k] = 0.0;
      ne = luf_vt_sparse(inv->luf, ne, ind, work);
      for (t = 1; t <= ne; t++) vec[ind[t]] = work[t];
      ne = inv_ht_sparse(inv, ne, ind, vec);
      for (t = 1; t <= ne; t++)
         k = ind[t], work[t] = vec[k], vec[k] = 0.0;
      inv->luf->pp_row = p0_row;
      inv->luf->pp_col = p0_col;
      ne = luf_ft_sparse(inv->luf, ne, ind, work);
      inv->luf->pp_row = pp_row;
      inv->luf->pp_col = pp_col;
      for (t = 1; t <= ne; t++) vec[ind[t]] = work[t];
      return ne;
}

/* eof */
