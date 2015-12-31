/* glplpx02.c (problem retrieving routines) */

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

#include "glpapi.h"
#include "glplib.h"
#include "glpk.h"
#define fault xfault1

/*----------------------------------------------------------------------
-- glp_get_prob_name - retrieve problem name.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- const char *glp_get_prob_name(glp_prob *lp);
--
-- *Returns*
--
-- The routine glp_get_prob_name returns a pointer to a static buffer,
-- which contains symbolic name of the problem. However, if the problem
-- has no assigned name, the routine returns NULL. */

const char *glp_get_prob_name(glp_prob *lp)
{     char *name;
      if (lp->name == NULL)
         name = NULL;
      else
         name = get_str(lp->str_buf, lp->name);
      return name;
}

#if 0
/*----------------------------------------------------------------------
-- lpx_get_class - retrieve problem class.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_get_class(glp_prob *lp);
--
-- *Returns*
--
-- The routine lpx_get_class returns a problem class for the specified
-- problem object:
--
-- LPX_LP  - pure linear programming (LP) problem;
-- LPX_MIP - mixed integer programming (MIP) problem. */

int lpx_get_class(glp_prob *lp)
{     int klass;
#if 0
      klass = lp->klass;
#else
      klass = (lpx_get_num_int(lp) == 0 ? LPX_LP : LPX_MIP);
#endif
      return klass;
}
#endif

/*----------------------------------------------------------------------
-- glp_get_obj_name - retrieve objective function name.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- const char *glp_get_obj_name(glp_prob *lp);
--
-- *Returns*
--
-- The routine glp_get_obj_name returns a pointer to a static buffer,
-- which contains a symbolic name of the objective function. However,
-- if the objective function has no assigned name, the routine returns
-- NULL. */

const char *glp_get_obj_name(glp_prob *lp)
{     char *name;
      if (lp->obj == NULL)
         name = NULL;
      else
         name = get_str(lp->str_buf, lp->obj);
      return name;
}

/*----------------------------------------------------------------------
-- glp_get_obj_dir - retrieve optimization direction flag.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int glp_get_obj_dir(glp_prob *lp);
--
-- *Returns*
--
-- The routine glp_get_obj_dir returns the optimization direction flag
-- (i.e. "sense" of the objective function):
--
-- GLP_MIN - minimization;
-- GLP_MAX - maximization. */

int glp_get_obj_dir(glp_prob *lp)
{     int dir;
      dir = lp->dir;
      return dir;
}

/*----------------------------------------------------------------------
-- glp_get_num_rows - retrieve number of rows.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int glp_get_num_rows(glp_prob *lp);
--
-- *Returns*
--
-- The routine glp_get_num_rows returns the current number of rows in
-- a problem object, which the parameter lp points to. */

int glp_get_num_rows(glp_prob *lp)
{     int m;
      m = lp->m;
      return m;
}

/*----------------------------------------------------------------------
-- glp_get_num_cols - retrieve number of columns.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int glp_get_num_cols(glp_prob *lp);
--
-- *Returns*
--
-- The routine glp_get_num_cols returns the current number of columns
-- in a problem object, which the parameter lp points to. */

int glp_get_num_cols(glp_prob *lp)
{     int n;
      n = lp->n;
      return n;
}

/*----------------------------------------------------------------------
-- glp_get_num_int - retrieve number of integer columns.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int glp_get_num_int(glp_prob *lp);
--
-- *Returns*
--
-- The routine glp_get_num_int returns the current number of columns,
-- which are marked as integer. */

int glp_get_num_int(glp_prob *lp)
{     LPXCOL *col;
      int j, count;
      count = 0;
      for (j = 1; j <= lp->n; j++)
      {  col = lp->col[j];
         if (col->kind == GLP_IV) count++;
      }
      return count;
}

/*----------------------------------------------------------------------
-- glp_get_num_bin - retrieve number of binary columns.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int glp_get_num_bin(glp_prob *lp);
--
-- *Returns*
--
-- The routine glp_get_num_bin returns the current number of columns,
-- which are marked as binary. */

int glp_get_num_bin(glp_prob *lp)
{     LPXCOL *col;
      int j, count;
      count = 0;
      for (j = 1; j <= lp->n; j++)
      {  col = lp->col[j];
         if (col->kind == GLP_IV &&
            (col->type == GLP_DB && col->lb == 0.0 && col->ub == 1.0))
            count++;
      }
      return count;
}

/*----------------------------------------------------------------------
-- glp_get_row_name - retrieve row name.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- const char *glp_get_row_name(glp_prob *lp, int i);
--
-- *Returns*
--
-- The routine glp_get_row_name returns a pointer to a static buffer,
-- which contains symbolic name of i-th row. However, if i-th row has
-- no assigned name, the routine returns NULL. */

const char *glp_get_row_name(glp_prob *lp, int i)
{     char *name;
      if (!(1 <= i && i <= lp->m))
         fault("glp_get_row_name: i = %d; row number out of range", i);
      if (lp->row[i]->name == NULL)
         name = NULL;
      else
         name = get_str(lp->str_buf, lp->row[i]->name);
      return name;
}

/*----------------------------------------------------------------------
-- glp_get_col_name - retrieve column name.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- const char *glp_get_col_name(glp_prob *lp, int j);
--
-- *Returns*
--
-- The routine glp_get_col_name returns a pointer to a static buffer,
-- which contains symbolic name of j-th column. However, if j-th column
-- has no assigned name, the routine returns NULL. */

const char *glp_get_col_name(glp_prob *lp, int j)
{     char *name;
      if (!(1 <= j && j <= lp->n))
         fault("glp_get_col_name: j = %d; column number out of range",
            j);
      if (lp->col[j]->name == NULL)
         name = NULL;
      else
         name = get_str(lp->str_buf, lp->col[j]->name);
      return name;
}

/*----------------------------------------------------------------------
-- glp_get_col_kind - retrieve column kind.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int glp_get_col_kind(glp_prob *lp, int j);
--
-- *Returns*
--
-- The routine glp_get_col_kind returns the kind of j-th column, i.e.
-- the kind of corresponding structural variable, as follows:
--
-- GLP_CV - continuous variable;
-- GLP_IV - integer variable;
-- GLP_BV - binary variable */

int glp_get_col_kind(glp_prob *lp, int j)
{     LPXCOL *col;
      int kind;
      if (!(1 <= j && j <= lp->n))
         fault("glp_get_col_kind: j = %d; column number out of range",
            j);
      col = lp->col[j];
      if (col->kind == GLP_CV)
         kind = GLP_CV;
else if (!(col->type == GLP_DB && col->lb == 0.0 && col->ub == 1.0))
         kind = GLP_IV;
      else
         kind = GLP_BV;
      return kind;
}

/*----------------------------------------------------------------------
-- glp_get_row_type - retrieve row type.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int glp_get_row_type(glp_prob *lp, int i);
--
-- *Returns*
--
-- The routine glp_get_row_type returns the type of i-th row, i.e. the
-- type of corresponding auxiliary variable, as follows:
--
-- GLP_FR - free (unbounded) variable;
-- GLP_LO - variable with lower bound;
-- GLP_UP - variable with upper bound;
-- GLP_DB - double-bounded variable;
-- GLP_FX - fixed variable. */

int glp_get_row_type(glp_prob *lp, int i)
{     if (!(1 <= i && i <= lp->m))
         fault("glp_get_row_type: i = %d; row number out of range", i);
      return lp->row[i]->type;
}

/*----------------------------------------------------------------------
-- glp_get_row_lb - retrieve row lower bound.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double glp_get_row_lb(glp_prob *lp, int i);
--
-- *Returns*
--
-- The routine glp_get_row_lb returns the lower bound of i-th row, i.e.
-- the lower bound of corresponding auxiliary variable. However, if the
-- row has no lower bound, the routine returns -DBL_MAX. */

double glp_get_row_lb(glp_prob *lp, int i)
{     double lb;
      if (!(1 <= i && i <= lp->m))
         fault("glp_get_row_lb: i = %d; row number out of range", i);
      switch (lp->row[i]->type)
      {  case GLP_FR:
         case GLP_UP:
            lb = -DBL_MAX; break;
         case GLP_LO:
         case GLP_DB:
         case GLP_FX:
            lb = lp->row[i]->lb; break;
         default:
            xassert(lp != lp);
      }
      return lb;
}

/*----------------------------------------------------------------------
-- lpx_get_row_lb - retrieve row lower bound.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double lpx_get_row_lb(glp_prob *lp, int i);
--
-- *Returns*
--
-- The routine lpx_get_row_lb returns the lower bound of i-th row, i.e.
-- the lower bound of corresponding auxiliary variable. However, if the
-- row has no lower bound, the routine returns zero. */

double lpx_get_row_lb(glp_prob *lp, int i)
{     if (!(1 <= i && i <= lp->m))
         fault("lpx_get_row_lb: i = %d; row number out of range", i);
      return lp->row[i]->lb;
}

/*----------------------------------------------------------------------
-- glp_get_row_ub - retrieve row upper bound.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double glp_get_row_ub(glp_prob *lp, int i);
--
-- *Returns*
--
-- The routine glp_get_row_ub returns the upper bound of i-th row, i.e.
-- the upper bound of corresponding auxiliary variable. However, if the
-- row has no upper bound, the routine returns +DBL_MAX. */

double glp_get_row_ub(glp_prob *lp, int i)
{     double ub;
      if (!(1 <= i && i <= lp->m))
         fault("glp_get_row_ub: i = %d; row number out of range", i);
      switch (lp->row[i]->type)
      {  case GLP_FR:
         case GLP_LO:
            ub = +DBL_MAX; break;
         case GLP_UP:
         case GLP_DB:
         case GLP_FX:
            ub = lp->row[i]->ub; break;
         default:
            xassert(lp != lp);
      }
      return ub;
}

/*----------------------------------------------------------------------
-- lpx_get_row_ub - retrieve row upper bound.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double lpx_get_row_ub(glp_prob *lp, int i);
--
-- *Returns*
--
-- The routine lpx_get_row_ub returns the upper bound of i-th row, i.e.
-- the upper bound of corresponding auxiliary variable. However, if the
-- row has no upper bound, the routine returns zero. */

double lpx_get_row_ub(glp_prob *lp, int i)
{     if (!(1 <= i && i <= lp->m))
         fault("lpx_get_row_ub: i = %d; row number out of range", i);
      return lp->row[i]->ub;
}

/*----------------------------------------------------------------------
-- glp_get_col_type - retrieve column type.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int glp_get_col_type(glp_prob *lp, int j);
--
-- *Returns*
--
-- The routine glp_get_col_type returns the type of j-th column, i.e.
-- the type of corresponding structural variable, as follows:
--
-- GLP_FR - free (unbounded) variable;
-- GLP_LO - variable with lower bound;
-- GLP_UP - variable with upper bound;
-- GLP_DB - double-bounded variable;
-- GLP_FX - fixed variable. */

int glp_get_col_type(glp_prob *lp, int j)
{     if (!(1 <= j && j <= lp->n))
         fault("glp_get_col_type: j = %d; column number out of range",
            j);
      return lp->col[j]->type;
}

/*----------------------------------------------------------------------
-- glp_get_col_lb - retrieve column lower bound.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double glp_get_col_lb(glp_prob *lp, int j);
--
-- *Returns*
--
-- The routine glp_get_col_lb returns the lower bound of j-th column,
-- i.e. the lower bound of corresponding structural variable. However,
-- if the column has no lower bound, the routine returns -DBL_MAX. */

double glp_get_col_lb(glp_prob *lp, int j)
{     double lb;
      if (!(1 <= j && j <= lp->n))
         fault("glp_get_col_lb: j = %d; column number out of range", j);
      switch (lp->col[j]->type)
      {  case GLP_FR:
         case GLP_UP:
            lb = -DBL_MAX; break;
         case GLP_LO:
         case GLP_DB:
         case GLP_FX:
            lb = lp->col[j]->lb; break;
         default:
            xassert(lp != lp);
      }
      return lb;
}

/*----------------------------------------------------------------------
-- lpx_get_col_lb - retrieve column lower bound.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double lpx_get_col_lb(glp_prob *lp, int j);
--
-- *Returns*
--
-- The routine lpx_get_col_lb returns the lower bound of j-th column,
-- i.e. the lower bound of corresponding structural variable. However,
-- if the column has no lower bound, the routine returns zero. */

double lpx_get_col_lb(glp_prob *lp, int j)
{     if (!(1 <= j && j <= lp->n))
         fault("lpx_get_col_lb: j = %d; column number out of range", j);
      return lp->col[j]->lb;
}

/*----------------------------------------------------------------------
-- glp_get_col_ub - retrieve column upper bound.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double glp_get_col_ub(glp_prob *lp, int j);
--
-- *Returns*
--
-- The routine glp_get_col_ub returns the upper bound of j-th column,
-- i.e. the upper bound of corresponding structural variable. However,
-- if the column has no upper bound, the routine returns +DBL_MAX. */

double glp_get_col_ub(glp_prob *lp, int j)
{     double ub;
      if (!(1 <= j && j <= lp->n))
         fault("glp_get_col_ub: j = %d; column number out of range", j);
      switch (lp->col[j]->type)
      {  case GLP_FR:
         case GLP_LO:
            ub = +DBL_MAX; break;
         case GLP_UP:
         case GLP_DB:
         case GLP_FX:
            ub = lp->col[j]->ub; break;
         default:
            xassert(lp != lp);
      }
      return ub;
}

/*----------------------------------------------------------------------
-- lpx_get_col_ub - retrieve column upper bound.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double lpx_get_col_ub(glp_prob *lp, int j);
--
-- *Returns*
--
-- The routine lpx_get_col_ub returns the upper bound of j-th column,
-- i.e. the upper bound of corresponding structural variable. However,
-- if the column has no upper bound, the routine returns zero. */

double lpx_get_col_ub(glp_prob *lp, int j)
{     if (!(1 <= j && j <= lp->n))
         fault("lpx_get_col_ub: j = %d; column number out of range", j);
      return lp->col[j]->ub;
}

/*----------------------------------------------------------------------
-- glp_get_obj_coef - retrieve obj. coefficient or constant term.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double glp_get_obj_coef(glp_prob *lp, int j);
--
-- *Returns*
--
-- The routine glp_get_obj_coef returns the objective coefficient at
-- j-th structural variable (column) of the specified problem object.
-- However, if the parameter j is 0, the routine returns the constant
-- term (i.e. "shift") of the objective function. */

double glp_get_obj_coef(glp_prob *lp, int j)
{     if (!(0 <= j && j <= lp->n))
         fault("glp_get_obj_coef: j = %d; column number out of range",
            j);
      return j == 0 ? lp->c0 : lp->col[j]->coef;
}

/*----------------------------------------------------------------------
-- glp_get_num_nz - retrieve number of constraint coefficients.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int glp_get_num_nz(glp_prob *lp);
--
-- *Returns*
--
-- The routine glp_get_num_nz returns the number of (non-zero) elements
-- in the constraint matrix of the specified problem object. */

int glp_get_num_nz(glp_prob *lp)
{     xassert(lp->nz == lp->aij_pool->count);
      return lp->nz;
}

/*----------------------------------------------------------------------
-- glp_get_mat_row - retrieve row of the constraint matrix.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int glp_get_mat_row(glp_prob *lp, int i, int ind[], double val[]);
--
-- *Description*
--
-- The routine glp_get_mat_row scans (non-zero) elements of i-th row
-- of the constraint matrix of the specified problem object and stores
-- their column indices and numeric values to locations ind[1], ...,
-- ind[len] and val[1], ..., val[len], respectively, where 0 <= len <= n
-- is the number of elements in i-th row, n is the number of columns.
--
-- The parameter ind or/and val can be specified as NULL, in which case
-- corresponding information is not stored.
--
-- *Returns*
--
-- The routine glp_get_mat_row returns the length len, i.e. the number
-- of (non-zero) elements in i-th row. */

int glp_get_mat_row(glp_prob *lp, int i, int ind[], double val[])
{     LPXAIJ *aij;
      int len;
      if (!(1 <= i && i <= lp->m))
         fault("glp_get_mat_row: i = %d; row number out of range", i);
      len = 0;
      for (aij = lp->row[i]->ptr; aij != NULL; aij = aij->r_next)
      {  len++;
         if (ind != NULL) ind[len] = aij->col->j;
         if (val != NULL) val[len] = aij->val;
      }
      xassert(len <= lp->n);
      return len;
}

/*----------------------------------------------------------------------
-- glp_get_mat_col - retrieve column of the constraint matrix.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int glp_get_mat_col(glp_prob *lp, int j, int ind[], double val[]);
--
-- *Description*
--
-- The routine glp_get_mat_col scans (non-zero) elements of j-th column
-- of the constraint matrix of the specified problem object and stores
-- their row indices and numeric values to locations ind[1], ...,
-- ind[len] and val[1], ..., val[len], respectively, where 0 <= len <= m
-- is the number of elements in j-th column, m is the number of rows.
--
-- The parameter ind or/and val can be specified as NULL, in which case
-- corresponding information is not stored.
--
-- *Returns*
--
-- The routine glp_get_mat_col returns the length len, i.e. the number
-- of (non-zero) elements in j-th column. */

int glp_get_mat_col(glp_prob *lp, int j, int ind[], double val[])
{     LPXAIJ *aij;
      int len;
      if (!(1 <= j && j <= lp->n))
         fault("glp_get_mat_col: j = %d; column number out of range",
            j);
      len = 0;
      for (aij = lp->col[j]->ptr; aij != NULL; aij = aij->c_next)
      {  len++;
         if (ind != NULL) ind[len] = aij->row->i;
         if (val != NULL) val[len] = aij->val;
      }
      xassert(len <= lp->m);
      return len;
}

/*----------------------------------------------------------------------
-- lpx_get_rii - retrieve row scale factor.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double lpx_get_rii(glp_prob *lp, int i);
--
-- *Returns*
--
-- The routine lpx_get_rii returns current scale factor r[i,i] for i-th
-- row of the specified problem object. */

double lpx_get_rii(glp_prob *lp, int i)
{     if (!(1 <= i && i <= lp->m))
         fault("lpx_get_rii: i = %d; row number out of range", i);
      return lp->row[i]->rii;
}

/*----------------------------------------------------------------------
-- lpx_get_sjj - retrieve column scale factor.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double lpx_get_sjj(glp_prob *lp, int j);
--
-- *Returns*
--
-- The routine lpx_get_sjj returns current scale factor s[j,j] for j-th
-- column of the specified problem object. */

double lpx_get_sjj(glp_prob *lp, int j)
{     if (!(1 <= j && j <= lp->n))
         fault("lpx_get_sjj: j = %d; column number out of range", j);
      return lp->col[j]->sjj;
}

/*----------------------------------------------------------------------
-- lpx_is_b_avail - check if LP basis is available.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_is_b_avail(glp_prob *lp);
--
-- *Returns*
--
-- If the LP basis associated with the specified problem object exists
-- and therefore available for computations, the routine lpx_is_b_avail
-- returns non-zero. Otherwise, if the LP basis is not available, the
-- routine returns zero. */

int lpx_is_b_avail(glp_prob *lp)
{     int avail;
#if 0
      switch (lp->b_stat)
      {  case LPX_B_UNDEF:
            avail = 0;
            break;
         case LPX_B_VALID:
            avail = 1;
#if 1
            xassert(bfi_is_valid(lp->b_inv));
#endif
            break;
         default:
            xassert(lp != lp);
      }
#else
      avail = bfi_is_valid(lp->b_inv);
#endif
      return avail;
}

/*----------------------------------------------------------------------
-- lpx_get_b_info - retrieve LP basis information.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_get_b_info(glp_prob *lp, int i);
--
-- *Returns*
--
-- The routine lpx_get_b_info returns the ordinal number k of auxiliary
-- (1 <= k <= m) or structural (m+1 <= k <= m+n) variable, which is
-- basic variable xB[i], 1 <= i <= m, in the current basis associated
-- with the specified problem object, where m is the number of rows and
-- n is the number of columns. */

int lpx_get_b_info(glp_prob *lp, int i)
{     if (!lpx_is_b_avail(lp))
         fault("lpx_get_b_info: LP basis is not available");
      if (!(1 <= i && i <= lp->m))
         fault("lpx_get_b_info: i = %d; index out of range", i);
      return lp->basis[i];
}

/*----------------------------------------------------------------------
-- lpx_get_row_b_ind - retrieve row index in LP basis.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_get_row_b_ind(glp_prob *lp, int i);
--
-- *Returns*
--
-- The routine lpx_get_row_b_ind returns the index k of basic variable
-- xB[k], 1 <= k <= m, which is the auxiliary variable associated with
-- i-th row in the current basis for the specified problem object.
-- However, if the auxiliary variable is non-basic, the routine returns
-- zero. */

int lpx_get_row_b_ind(glp_prob *lp, int i)
{     if (!lpx_is_b_avail(lp))
         fault("lpx_get_row_b_ind: LP basis is not available");
      if (!(1 <= i && i <= lp->m))
         fault("lpx_get_row_b_ind: i = %d; row number out of range", i);
      return lp->row[i]->b_ind;
}

/*----------------------------------------------------------------------
-- lpx_get_col_b_ind - retrieve column index in LP basis.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_get_col_b_ind(glp_prob *lp, int j);
--
-- *Returns*
--
-- The routine lpx_get_col_b_ind returns the index k of basic variable
-- xB[k], 1 <= k <= m, which is the structural variable associated with
-- j-th column in the current basis for the specified problem object.
-- However, if the structural variable is non-basic, the routine returns
-- zero. */

int lpx_get_col_b_ind(glp_prob *lp, int j)
{     if (!lpx_is_b_avail(lp))
         fault("lpx_get_col_b_ind: LP basis is not available");
      if (!(1 <= j && j <= lp->n))
         fault("lpx_get_col_b_ind: j = %d; column number out of range",
            j);
      return lp->col[j]->b_ind;
}

/*----------------------------------------------------------------------
-- lpx_access_inv - access factorization of basis matrix.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- BFI *lpx_access_inv(glp_prob *lp);
--
-- *Returns*
--
-- The routine lpx_access_inv returns a pointer to a factorization of
-- the basis matrix for the specified problem object.
--
-- NOTE: This routine is intended for internal use only. */

BFI *lpx_access_inv(glp_prob *lp)
{     BFI *b_inv;
      b_inv = lp->b_inv;
      return b_inv;
}

/*----------------------------------------------------------------------
-- lpx_get_status - retrieve generic status of basic solution.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_get_status(glp_prob *lp);
--
-- *Returns*
--
-- The routine lpx_get_status reports the generic status of the basic
-- solution for the specified problem object as follows:
--
-- LPX_OPT    - solution is optimal;
-- LPX_FEAS   - solution is feasible;
-- LPX_INFEAS - solution is infeasible;
-- LPX_NOFEAS - problem has no feasible solution;
-- LPX_UNBND  - problem has unbounded solution;
-- LPX_UNDEF  - solution is undefined. */

int lpx_get_status(glp_prob *lp)
{     int status;
      switch (lp->p_stat)
      {  case LPX_P_UNDEF:
            status = LPX_UNDEF;
            break;
         case LPX_P_FEAS:
            switch (lp->d_stat)
            {  case LPX_D_UNDEF:
                  status = LPX_FEAS;
                  break;
               case LPX_D_FEAS:
                  status = LPX_OPT;
                  break;
               case LPX_D_INFEAS:
                  status = LPX_FEAS;
                  break;
               case LPX_D_NOFEAS:
                  status = LPX_UNBND;
                  break;
               default:
                  xassert(lp != lp);
            }
            break;
         case LPX_P_INFEAS:
            status = LPX_INFEAS;
            break;
         case LPX_P_NOFEAS:
            status = LPX_NOFEAS;
            break;
         default:
            xassert(lp != lp);
      }
      return status;
}

/*----------------------------------------------------------------------
-- lpx_get_prim_stat - retrieve primal status of basic solution.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_get_prim_stat(glp_prob *lp);
--
-- *Returns*
--
-- The routine lpx_get_prim_stat reports the primal status of the basic
-- solution for the specified problem object as follows:
--
-- LPX_P_UNDEF  - primal solution is undefined;
-- LPX_P_FEAS   - solution is primal feasible;
-- LPX_P_INFEAS - solution is primal infeasible;
-- LPX_P_NOFEAS - no primal feasible solution exists. */

int lpx_get_prim_stat(glp_prob *lp)
{     int p_stat;
      p_stat = lp->p_stat;
      return p_stat;
}

/*----------------------------------------------------------------------
-- lpx_get_dual_stat - retrieve dual status of basic solution.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_get_dual_stat(glp_prob *lp);
--
-- *Returns*
--
-- The routine lpx_get_dual_stat reports the dual status of the basic
-- solution for the specified problem object as follows:
--
-- LPX_D_UNDEF  - dual solution is undefined;
-- LPX_D_FEAS   - solution is dual feasible;
-- LPX_D_INFEAS - solution is dual infeasible;
-- LPX_D_NOFEAS - no dual feasible solution exists. */

int lpx_get_dual_stat(glp_prob *lp)
{     int d_stat;
      d_stat = lp->d_stat;
      return d_stat;
}

/*----------------------------------------------------------------------
-- glp_get_obj_val - retrieve objective value (basic solution).
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double glp_get_obj_val(glp_prob *lp);
--
-- *Returns*
--
-- The routine glp_get_obj_val returns value of the objective function
-- for basic solution. */

double glp_get_obj_val(glp_prob *lp)
{     double z;
      z = lp->b_obj;
      if (lp->cps->round && fabs(z) < 1e-9) z = 0.0;
      return z;
}

/*----------------------------------------------------------------------
-- glp_get_row_stat - retrieve row status (basic solution).
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int glp_get_row_stat(glp_prob *lp, int i);
--
-- *Returns*
--
-- The routine glp_get_row_stat returns current status assigned to the
-- auxiliary variable associated with i-th row as follows:
--
-- GLP_BS - basic variable;
-- GLP_NL - non-basic variable on its lower bound;
-- GLP_NU - non-basic variable on its upper bound;
-- GLP_NF - non-basic free (unbounded) variable;
-- GLP_NS - non-basic fixed variable. */

int glp_get_row_stat(glp_prob *lp, int i)
{     if (!(1 <= i && i <= lp->m))
         fault("glp_get_row_stat: i = %d; row number out of range", i);
      return lp->row[i]->stat;
}

/*----------------------------------------------------------------------
-- glp_get_row_prim - retrieve row primal value (basic solution).
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double glp_get_row_prim(glp_prob *lp, int i);
--
-- *Returns*
--
-- The routine glp_get_row_prim returns primal value of the auxiliary
-- variable associated with i-th row. */

double glp_get_row_prim(glp_prob *lp, int i)
{     double prim;
      if (!(1 <= i && i <= lp->m))
         fault("glp_get_row_prim: i = %d; row number out of range", i);
      prim = lp->row[i]->prim;
      if (lp->cps->round && fabs(prim) < 1e-9) prim = 0.0;
      return prim;
}

/*----------------------------------------------------------------------
-- glp_get_row_dual - retrieve row dual value (basic solution).
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double glp_get_row_dual(glp_prob *lp, int i);
--
-- *Returns*
--
-- The routine glp_get_row_dual returns dual value (i.e. reduced cost)
-- of the auxiliary variable associated with i-th row. */

double glp_get_row_dual(glp_prob *lp, int i)
{     double dual;
      if (!(1 <= i && i <= lp->m))
         fault("glp_get_row_dual: i = %d; row number out of range", i);
      dual = lp->row[i]->dual;
      if (lp->cps->round && fabs(dual) < 1e-9) dual = 0.0;
      return dual;
}

/*----------------------------------------------------------------------
-- glp_get_col_stat - retrieve column status (basic solution).
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int glp_get_col_stat(glp_prob *lp, int j);
--
-- *Returns*
--
-- The routine glp_get_col_stat returns current status assigned to the
-- structural variable associated with j-th column as follows:
--
-- GLP_BS - basic variable;
-- GLP_NL - non-basic variable on its lower bound;
-- GLP_NU - non-basic variable on its upper bound;
-- GLP_NF - non-basic free (unbounded) variable;
-- GLP_NS - non-basic fixed variable. */

int glp_get_col_stat(glp_prob *lp, int j)
{     if (!(1 <= j && j <= lp->n))
         fault("glp_get_col_stat: j = %d; column number out of range",
            j);
      return lp->col[j]->stat;
}

/*----------------------------------------------------------------------
-- glp_get_col_prim - retrieve column primal value (basic solution).
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double glp_get_col_prim(glp_prob *lp, int j);
--
-- *Returns*
--
-- The routine glp_get_col_prim returns primal value of the structural
-- variable associated with j-th column. */

double glp_get_col_prim(glp_prob *lp, int j)
{     double prim;
      if (!(1 <= j && j <= lp->n))
         fault("glp_get_col_prim: j = %d; column number out of range",
            j);
      prim = lp->col[j]->prim;
      if (lp->cps->round && fabs(prim) < 1e-9) prim = 0.0;
      return prim;
}

/*----------------------------------------------------------------------
-- glp_get_col_dual - retrieve column dual value (basic solution).
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double glp_get_col_dual(glp_prob *lp, int j);
--
-- *Returns*
--
-- The routine glp_get_col_dual returns dual value (i.e. reduced cost)
-- of the structural variable associated with j-th column. */

double glp_get_col_dual(glp_prob *lp, int j)
{     double dual;
      if (!(1 <= j && j <= lp->n))
         fault("glp_get_col_dual: j = %d; column number out of range",
            j);
      dual = lp->col[j]->dual;
      if (lp->cps->round && fabs(dual) < 1e-9) dual = 0.0;
      return dual;
}

/*----------------------------------------------------------------------
-- lpx_get_ray_info - retrieve row/column which causes unboundness.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_get_ray_info(glp_prob *lp);
--
-- *Returns*
--
-- The routine lpx_get_ray_info returns the number k of some non-basic
-- variable x[k], which causes primal unboundness. If such a variable
-- cannot be identified, the routine returns zero.
--
-- If 1 <= k <= m, x[k] is the auxiliary variable associated with k-th
-- row, if m+1 <= k <= m+n, x[k] is the structural variable associated
-- with (k-m)-th column, where m is the number of rows, n is the number
-- of columns in the LP problem object. */

int lpx_get_ray_info(glp_prob *lp)
{     int k;
      k = lp->some;
      return k;
}

/*----------------------------------------------------------------------
-- lpx_ipt_status - retrieve status of interior-point solution.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_ipt_status(glp_prob *lp);
--
-- *Returns*
--
-- The routine lpx_ipt_status reports the status of solution found by
-- interior-point solver as follows:
--
-- LPX_T_UNDEF - interior-point solution is undefined;
-- LPX_T_OPT   - interior-point solution is optimal. */

int lpx_ipt_status(glp_prob *lp)
{     int t_stat;
      t_stat = lp->t_stat;
      return t_stat;
}

/*----------------------------------------------------------------------
-- glp_ipt_obj_val - retrieve objective value (interior point).
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double glp_ipt_obj_val(glp_prob *lp);
--
-- *Returns*
--
-- The routine glp_ipt_obj_val returns value of the objective function
-- for interior-point solution. */

double glp_ipt_obj_val(glp_prob *lp)
{     double z;
      z = lp->t_obj;
      if (lp->cps->round && fabs(z) < 1e-9) z = 0.0;
      return z;
}

/*----------------------------------------------------------------------
-- glp_ipt_row_prim - retrieve row primal value (interior point).
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double glp_ipt_row_prim(glp_prob *lp, int i);
--
-- *Returns*
--
-- The routine glp_ipt_row_prim returns primal value of the auxiliary
-- variable associated with i-th row. */

double glp_ipt_row_prim(glp_prob *lp, int i)
{     double pval;
      if (!(1 <= i && i <= lp->m))
         fault("glp_ipt_row_prim: i = %d; row number out of range", i);
      pval = lp->row[i]->pval;
      if (lp->cps->round && fabs(pval) < 1e-9) pval = 0.0;
      return pval;
}

/*----------------------------------------------------------------------
-- glp_ipt_row_dual - retrieve row dual value (interior point).
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double glp_ipt_row_dual(glp_prob *lp, int i);
--
-- *Returns*
--
-- The routine glp_ipt_row_dual returns dual value (i.e. reduced cost)
-- of the auxiliary variable associated with i-th row. */

double glp_ipt_row_dual(glp_prob *lp, int i)
{     double dval;
      if (!(1 <= i && i <= lp->m))
         fault("glp_ipt_row_dual: i = %d; row number out of range", i);
      dval = lp->row[i]->dval;
      if (lp->cps->round && fabs(dval) < 1e-9) dval = 0.0;
      return dval;
}

/*----------------------------------------------------------------------
-- glp_ipt_col_prim - retrieve column primal value (interior point).
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double glp_ipt_col_prim(glp_prob *lp, int j);
--
-- *Returns*
--
-- The routine glp_ipt_col_prim returns primal value of the structural
-- variable associated with j-th column. */

double glp_ipt_col_prim(glp_prob *lp, int j)
{     double pval;
      if (!(1 <= j && j <= lp->n))
         fault("glp_ipt_col_prim: j = %d; column number out of range",
            j);
      pval = lp->col[j]->pval;
      if (lp->cps->round && fabs(pval) < 1e-9) pval = 0.0;
      return pval;
}

/*----------------------------------------------------------------------
-- glp_ipt_col_dual - retrieve column dual value (interior point).
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double glp_ipt_col_dual(glp_prob *lp, int j);
--
-- *Returns*
--
-- The routine glp_ipt_col_dual returns dual value (i.e. reduced cost)
-- of the structural variable associated with j-th column. */

double glp_ipt_col_dual(glp_prob *lp, int j)
{     double dval;
      if (!(1 <= j && j <= lp->n))
         fault("glp_ipt_col_dual: j = %d; column number out of range",
            j);
      dval = lp->col[j]->dval;
      if (lp->cps->round && fabs(dval) < 1e-9) dval = 0.0;
      return dval;
}

/*----------------------------------------------------------------------
-- lpx_mip_status - retrieve status of MIP solution.
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- int lpx_mip_status(glp_prob *lp);
--
-- *Returns*
--
-- The routine lpx_mip_status reports the status of MIP solution found
-- by branch-and-bound solver as follows:
--
-- LPX_I_UNDEF  - MIP solution is undefined;
-- LPX_I_OPT    - MIP solution is integer optimal;
-- LPX_I_FEAS   - MIP solution is integer feasible but its optimality
--                (or non-optimality) has not been proven, perhaps due
--                to premature termination of the search;
-- LPX_I_NOFEAS - problem has no integer feasible solution (proven by
--                the solver). */

int lpx_mip_status(glp_prob *lp)
{     int i_stat;
#if 0
      if (lp->klass != LPX_MIP)
         fault("lpx_mip_status: not a MIP problem");
#endif
      i_stat = lp->i_stat;
      return i_stat;
}

/*----------------------------------------------------------------------
-- glp_mip_obj_val - retrieve objective value (MIP solution).
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double glp_mip_obj_val(glp_prob *lp);
--
-- *Returns*
--
-- The routine glp_mip_obj_val returns value of the objective function
-- for MIP solution. */

double glp_mip_obj_val(glp_prob *lp)
{     double z;
      z = lp->i_obj;
      if (lp->cps->round && fabs(z) < 1e-9) z = 0.0;
      return z;
}

/*----------------------------------------------------------------------
-- glp_mip_row_val - retrieve row value (MIP solution).
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double glp_mip_row_val(glp_prob *lp, int i);
--
-- *Returns*
--
-- The routine glp_mip_row_val returns value of the auxiliary variable
-- associated with i-th row. */

double glp_mip_row_val(glp_prob *lp, int i)
{     double mipx;
      if (!(1 <= i && i <= lp->m))
         fault("glp_mip_row_val: i = %d; row number out of range", i);
      mipx = lp->row[i]->mipx;
      if (lp->cps->round && fabs(mipx) < 1e-9) mipx = 0.0;
      return mipx;
}

/*----------------------------------------------------------------------
-- glp_mip_col_val - retrieve column value (MIP solution).
--
-- *Synopsis*
--
-- #include "glplpx.h"
-- double glp_mip_col_val(glp_prob *lp, int j);
--
-- *Returns*
--
-- The routine glp_mip_col_val returns value of the structural variable
-- associated with j-th column. */

double glp_mip_col_val(glp_prob *lp, int j)
{     double mipx;
      if (!(1 <= j && j <= lp->n))
         fault("glp_mip_col_val: j = %d; column number out of range",
            j);
      mipx = lp->col[j]->mipx;
      if (lp->cps->round && fabs(mipx) < 1e-9) mipx = 0.0;
      return mipx;
}

/*----------------------------------------------------------------------
-- Obsolete API routines are kept for backward compatibility and will
-- be removed in the future. */

void lpx_get_row_bnds(glp_prob *lp, int i, int *typx, double *lb,
      double *ub)
{     /* obtain row bounds */
      if (typx != NULL) *typx = lpx_get_row_type(lp, i);
      if (lb != NULL) *lb = lpx_get_row_lb(lp, i);
      if (ub != NULL) *ub = lpx_get_row_ub(lp, i);
      return;
}

void lpx_get_col_bnds(glp_prob *lp, int j, int *typx, double *lb,
      double *ub)
{     /* obtain column bounds */
      if (typx != NULL) *typx = lpx_get_col_type(lp, j);
      if (lb != NULL) *lb = lpx_get_col_lb(lp, j);
      if (ub != NULL) *ub = lpx_get_col_ub(lp, j);
      return;
}

void lpx_get_row_info(glp_prob *lp, int i, int *tagx, double *vx,
      double *dx)
{     /* obtain row solution information */
      if (tagx != NULL) *tagx = lpx_get_row_stat(lp, i);
      if (vx != NULL) *vx = lpx_get_row_prim(lp, i);
      if (dx != NULL) *dx = lpx_get_row_dual(lp, i);
      return;
}

void lpx_get_col_info(glp_prob *lp, int j, int *tagx, double *vx,
      double *dx)
{     /* obtain column solution information */
      if (tagx != NULL) *tagx = lpx_get_col_stat(lp, j);
      if (vx != NULL) *vx = lpx_get_col_prim(lp, j);
      if (dx != NULL) *dx = lpx_get_col_dual(lp, j);
      return;
}

/* eof */
