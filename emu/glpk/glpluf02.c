/* glpluf02.c */

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

#include "glplib.h"
#include "glpluf.h"
#define fault xfault1

#define _GLPLUF_DEBUG 0
/* turning this flag on may significantly degrade the performance */

/*----------------------------------------------------------------------
-- luf_v_sparse - solve system V*x = b (sparse version).
--
-- SYNOPSIS
--
-- #include "glpluf.h"
-- int luf_v_sparse(LUF *luf, int len, int ind[], double val[]);
--
-- DESCRIPTION
--
-- The routine luf_v_sparse solves the system V*x = b, where the matrix
-- V is a component of LU-factorization specified by the parameter luf.
--
-- On entry the parameters len, ind, and val should specify the
-- right-hand side vector b in sparse format. On exit these parameters
-- contain the solution vector x in sparse format.
--
-- WARNING: On entry the array pos should contain zeros in locations
--          pos[1], ..., pos[n]. The routine retains this condition on
--          exit.
--
-- RETURNS
--
-- The routine luf_v_sparse returns the number of structural non-zeros
-- in the solution vector x.
--
-- TECHNIQUE
--
-- The technique used in the routine is based on breadth-first search
-- to determine the subset of equations needed to compute the solution
-- and topological sort to determine the order in which the equations
-- should be solved. */

int luf_v_sparse(LUF *luf, int len, int ind[], double val[])
{     int *vc_ptr = luf->vc_ptr;
      int *vc_len = luf->vc_len;
      double *vr_piv = luf->vr_piv;
      int *pp_col = luf->pp_col;
      int *qq_col = luf->qq_col;
      int *sv_ndx = luf->sv_ndx;
      double *sv_val = luf->sv_val;
      int *pos = luf->wpos;
      int *cnt = luf->wcnt;
      int *que = luf->wque;
      int i, j, k, t, ptr, end, lll;
      double xj;
#if _GLPLUF_DEBUG
      /* on entry the array pos must be clear */
      for (i = 1; i <= luf->n; i++) insist(pos[i] == 0);
#endif
      /* determine the subset of equations which are needed to compute
         all structurally non-zero solution components */
      /* len      is the number of equations included into the subset;
         ind[t] = i means that equation i is included into the subset,
                  where 1 <= t <= len;
         pos[i] = t means that ind[t] = i;
         cnt[t]   is the number of structurally non-zero variables that
                  should be substituted into the corresponding equation
                  to compute its pivot variable */
      /* initially the subset includes equations whose right-hand sides
         being specified on entry are structurally non-zero */
      for (t = 1; t <= len; t++)
      {  i = ind[t];
#if _GLPLUF_DEBUG
         insist(1 <= i && i <= luf->n);
         /* as usual, duplicate rhs indices are not allowed */
         insist(pos[i] == 0);
#endif
         /* indicate that equation i is in the subset */
         pos[i] = t;
         /* and reset its incoming variables count */
         cnt[t] = 0;
      }
      /* perform breadth-first search to determine the complete subset
         of equations */
      for (t = 1; t <= len; t++)
      {  i = ind[t];
         /* determine pivot variable j for equation i */
         j = qq_col[pp_col[i]];
         /* equation i is in the subset, so its pivot variable j is
            structurally non-zero; therefore all other equations having
            this variable should be included in the subset */
         for (ptr = vc_ptr[j], end = ptr + vc_len[j]; ptr < end; ptr++)
         {  k = sv_ndx[ptr];
            if (pos[k] == 0)
            {  /* include equation k in the subset (note that its rhs
                  is zero) */
               len++;
               ind[len] = k;
               val[len] = 0.0;
               pos[k] = len;
               cnt[len] = 0;
            }
            /* equation k needs incoming variable j to be computed, so
               increase its incoming variables count */
            cnt[pos[k]]++;
         }
      }
      /* now the complete subsystem of equations has been determined */
      /* we cannot resolve an equation if it has at least one incoming
         variable whose value is still not computed from other equation;
         so we start resolving from equations with zero counts and then
         substitute just computed variables into other equations that
         decreases their counts until all equations have been resolved
         and the complete solution vector has been computed */
      /* perform initial selection of equations with zero count */
      lll = 0;
      for (t = 1; t <= len; t++)
         if (cnt[t] == 0) que[++lll] = t;
      /* walk through equation in the subsystem in the order determined
         by means of topological sorting */
      while (lll > 0)
      {  t = que[lll--];
         i = ind[t];
         /* equation i has zero count, i.e. all incoming variables of
            this equation have been computed and substituted, so we can
            compute the corresponding pivot variable j */
         j = qq_col[pp_col[i]];
         ind[t] = j;
         xj = (val[t] /= vr_piv[i]);
         /* equation i has been completely processed */
         pos[i] = 0;
         /* substitute variable j in other equations (this step cannot
            be passed even if the variable is numerically zero) */
         for (ptr = vc_ptr[j], end = ptr + vc_len[j]; ptr < end; ptr++)
         {  k = sv_ndx[ptr];
            /* substituting variable j into equation k means updating
               its right-hand side */
            t = pos[k];
            val[t] -= sv_val[ptr] * xj;
            /* decrease the incoming variables count */
            cnt[t]--;
            /* if it became zero, the equation can be resolved */
            if (cnt[t] == 0) que[++lll] = t;
         }
      }
#if _GLPLUF_DEBUG
      /* on exit the array pos must be clear */
      /* this check can fail only due to an error in the program logic
         or because the matrix graph has cycles, i.e. if the matrix is
         not triangular */
      for (i = 1; i <= luf->n; i++) insist(pos[i] == 0);
#endif
      return len;
}

/*----------------------------------------------------------------------
-- luf_vt_sparse - solve system V'*x = b (sparse version).
--
-- SYNOPSIS
--
-- #include "glpluf.h"
-- int luf_vt_sparse(LUF *luf, int len, int ind[], double val[]);
--
-- DESCRIPTION
--
-- The routine luf_vt_sparse solves the system V'*x = b, where V' is
-- a matrix transposed to the matrix V, a component of LU-factorization
-- specified by the parameter luf.
--
-- On entry the parameters len, ind, and val should specify the
-- right-hand side vector b in sparse format. On exit these parameters
-- contain the solution vector x in sparse format.
--
-- WARNING: On entry the array pos should contain zeros in locations
--          pos[1], ..., pos[n]. The routine retains this condition on
--          exit.
--
-- RETURNS
--
-- The routine luf_vt_sparse returns the number of structural non-zeros
-- in the solution vector x.
--
-- TECHNIQUE
--
-- See comments to the routine luf_v_sparse. */

/* This routine is a clone of the routine luf_v_sparse, where equations
   correspond to columns of V and variables correspond to rows of V, so
   all comments are omitted */

int luf_vt_sparse(LUF *luf, int len, int ind[], double val[])
{     int *vr_ptr = luf->vr_ptr;
      int *vr_len = luf->vr_len;
      double *vr_piv = luf->vr_piv;
      int *pp_row = luf->pp_row;
      int *qq_row = luf->qq_row;
      int *sv_ndx = luf->sv_ndx;
      double *sv_val = luf->sv_val;
      int *pos = luf->wpos;
      int *cnt = luf->wcnt;
      int *que = luf->wque;
      int i, j, k, t, ptr, end, lll;
      double xj;
#if _GLPLUF_DEBUG
      for (i = 1; i <= luf->n; i++) insist(pos[i] == 0);
#endif
      for (t = 1; t <= len; t++)
      {  i = ind[t];
#if _GLPLUF_DEBUG
         insist(1 <= i && i <= luf->n);
         insist(pos[i] == 0);
#endif
         pos[i] = t;
         cnt[t] = 0;
      }
      for (t = 1; t <= len; t++)
      {  i = ind[t];
         j = pp_row[qq_row[i]];
         for (ptr = vr_ptr[j], end = ptr + vr_len[j]; ptr < end; ptr++)
         {  k = sv_ndx[ptr];
            if (pos[k] == 0)
            {  len++;
               ind[len] = k;
               val[len] = 0.0;
               pos[k] = len;
               cnt[len] = 0;
            }
            cnt[pos[k]]++;
         }
      }
      lll = 0;
      for (t = 1; t <= len; t++)
         if (cnt[t] == 0) que[++lll] = t;
      while (lll > 0)
      {  t = que[lll--];
         i = ind[t];
         j = pp_row[qq_row[i]];
         ind[t] = j;
         xj = (val[t] /= vr_piv[j]);
         pos[i] = 0;
         for (ptr = vr_ptr[j], end = ptr + vr_len[j]; ptr < end; ptr++)
         {  k = sv_ndx[ptr];
            t = pos[k];
            val[t] -= sv_val[ptr] * xj;
            cnt[t]--;
            if (cnt[t] == 0) que[++lll] = t;
         }
      }
#if _GLPLUF_DEBUG
      for (i = 1; i <= luf->n; i++) insist(pos[i] == 0);
#endif
      return len;
}

/*----------------------------------------------------------------------
-- luf_f_sparse - solve system F*x = b (sparse version).
--
-- SYNOPSIS
--
-- #include "glpluf.h"
-- int luf_f_sparse(LUF *luf, int len, int ind[], double val[]);
--
-- DESCRIPTION
--
-- The routine luf_f_sparse solves the system F*x = b, where the matrix
-- F is a component of LU-factorization specified by the parameter luf.
--
-- On entry the parameters len, ind, and val should specify the
-- right-hand side vector b in sparse format. On exit these parameters
-- contain the solution vector x in sparse format.
--
-- WARNING: On entry the array pos should contain zeros in locations
--          pos[1], ..., pos[n]. The routine retains this condition on
--          exit.
--
-- RETURNS
--
-- The routine luf_f_sparse returns the number of structural non-zeros
-- in the solution vector x.
--
-- TECHNIQUE
--
-- See comments to the routine luf_v_sparse. */

/* This routine is a clone of the routine luf_v_sparse, where equations
   correspond to rows of F and variables correspond to columns of F, so
   all comments are omitted */

int luf_f_sparse(LUF *luf, int len, int ind[], double val[])
{     int *fc_ptr = luf->fc_ptr;
      int *fc_len = luf->fc_len;
      int *sv_ndx = luf->sv_ndx;
      double *sv_val = luf->sv_val;
      int *pos = luf->wpos;
      int *cnt = luf->wcnt;
      int *que = luf->wque;
      int i, j, k, t, ptr, end, lll;
      double xj;
#if _GLPLUF_DEBUG
      for (i = 1; i <= luf->n; i++) insist(pos[i] == 0);
#endif
      for (t = 1; t <= len; t++)
      {  i = ind[t];
#if _GLPLUF_DEBUG
         insist(1 <= i && i <= luf->n);
         insist(pos[i] == 0);
#endif
         pos[i] = t;
         cnt[t] = 0;
      }
      for (t = 1; t <= len; t++)
      {  i = ind[t];
         j = i; /* due to symmetric permutations of rows and columns */
         for (ptr = fc_ptr[j], end = ptr + fc_len[j]; ptr < end; ptr++)
         {  k = sv_ndx[ptr];
            if (pos[k] == 0)
            {  len++;
               ind[len] = k;
               val[len] = 0.0;
               pos[k] = len;
               cnt[len] = 0;
            }
            cnt[pos[k]]++;
         }
      }
      lll = 0;
      for (t = 1; t <= len; t++)
         if (cnt[t] == 0) que[++lll] = t;
      while (lll > 0)
      {  t = que[lll--];
         i = ind[t];
         j = i; /* due to symmetric permutations of rows and columns */
         xj = val[t];
         pos[i] = 0;
         for (ptr = fc_ptr[j], end = ptr + fc_len[j]; ptr < end; ptr++)
         {  k = sv_ndx[ptr];
            t = pos[k];
            val[t] -= sv_val[ptr] * xj;
            cnt[t]--;
            if (cnt[t] == 0) que[++lll] = t;
         }
      }
#if _GLPLUF_DEBUG
      for (i = 1; i <= luf->n; i++) insist(pos[i] == 0);
#endif
      return len;
}

/*----------------------------------------------------------------------
-- luf_ft_sparse - solve system F'*x = b (sparse version).
--
-- SYNOPSIS
--
-- #include "glpluf.h"
-- int luf_ft_sparse(LUF *luf, int len, int ind[], double val[]);
--
-- DESCRIPTION
--
-- The routine luf_ft_sparse solves the system F'*x = b, where F' is
-- a matrix transposed to the matrix F, a component of LU-factorization
-- specified by the parameter luf.
--
-- On entry the parameters len, ind, and val should specify the
-- right-hand side vector b in sparse format. On exit these parameters
-- contain the solution vector x in sparse format.
--
-- WARNING: On entry the array pos should contain zeros in locations
--          pos[1], ..., pos[n]. The routine retains this condition on
--          exit.
--
-- RETURNS
--
-- The routine luf_ft_sparse returns the number of structural non-zeros
-- in the solution vector x.
--
-- TECHNIQUE
--
-- See comments to the routine luf_v_sparse. */

/* This routine is a clone of the routine luf_v_sparse, where equations
   correspond to columns of F and variables correspond to rows of F, so
   all comments are omitted */

int luf_ft_sparse(LUF *luf, int len, int ind[], double val[])
{     int *fr_ptr = luf->fr_ptr;
      int *fr_len = luf->fr_len;
      int *sv_ndx = luf->sv_ndx;
      double *sv_val = luf->sv_val;
      int *pos = luf->wpos;
      int *cnt = luf->wcnt;
      int *que = luf->wque;
      int i, j, k, t, ptr, end, lll;
      double xj;
#if _GLPLUF_DEBUG
      for (i = 1; i <= luf->n; i++) insist(pos[i] == 0);
#endif
      for (t = 1; t <= len; t++)
      {  i = ind[t];
#if _GLPLUF_DEBUG
         insist(1 <= i && i <= luf->n);
         insist(pos[i] == 0);
#endif
         pos[i] = t;
         cnt[t] = 0;
      }
      for (t = 1; t <= len; t++)
      {  i = ind[t];
         j = i; /* due to symmetric permutations of rows and columns */
         for (ptr = fr_ptr[j], end = ptr + fr_len[j]; ptr < end; ptr++)
         {  k = sv_ndx[ptr];
            if (pos[k] == 0)
            {  len++;
               ind[len] = k;
               val[len] = 0.0;
               pos[k] = len;
               cnt[len] = 0;
            }
            cnt[pos[k]]++;
         }
      }
      lll = 0;
      for (t = 1; t <= len; t++)
         if (cnt[t] == 0) que[++lll] = t;
      while (lll > 0)
      {  t = que[lll--];
         i = ind[t];
         j = i; /* due to symmetric permutations of rows and columns */
         xj = val[t];
         pos[i] = 0;
         for (ptr = fr_ptr[j], end = ptr + fr_len[j]; ptr < end; ptr++)
         {  k = sv_ndx[ptr];
            t = pos[k];
            val[t] -= sv_val[ptr] * xj;
            cnt[t]--;
            if (cnt[t] == 0) que[++lll] = t;
         }
      }
#if _GLPLUF_DEBUG
      for (i = 1; i <= luf->n; i++) insist(pos[i] == 0);
#endif
      return len;
}

/*----------------------------------------------------------------------
-- luf_sparse - solve system A*x = b or A'*x = b (sparse version).
--
-- SYNOPSIS
--
-- #include "glpluf.h"
-- int luf_sparse(LUF *luf, int tr, int len, int ind[], double val[]);
--
-- DESCRIPTION
--
-- The routine luf_sparse solves either the system A*x = b (if the flag
-- tr is zero) or the system A'*x = b (if the flag tr is non-zero),
-- where the parameter luf specifies LU-factorization of the matrix A,
-- A' is a matrix transposed to A.
--
-- On entry the parameters len, ind, and val should specify the
-- right-hand side vector b in sparse format. On exit these parameters
-- contain the solution vector x in sparse format.
--
-- RETURNS
--
-- The routine luf_sparse returns the number of structural non-zeros in
-- the solution vector x. */

int luf_sparse(LUF *luf, int tr, int len, int ind[], double val[])
{     if (!luf->valid)
         fault("luf_sparse: LU-factorization is not valid");
      if (!tr)
      {  /* A = F*V, therefore inv(A) = inv(V)*inv(F) */
         len = luf_f_sparse(luf, len, ind, val);
         len = luf_v_sparse(luf, len, ind, val);
      }
      else
      {  /* A' = V'*F', therefore inv(A') = inv(F')*inv(V') */
         len = luf_vt_sparse(luf, len, ind, val);
         len = luf_ft_sparse(luf, len, ind, val);
      }
      return len;
}

/* eof */
