/* glpapi.h (application program interface) */

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

#ifndef _GLPAPI_H
#define _GLPAPI_H

#include "glpavl.h"
#include "glpbfi.h"
#include "glpstr.h"

/*----------------------------------------------------------------------
-- The structure LPX is an LP/MIP problem object, which corresponds to
-- the following problem statement:
--
--    minimize (or maximize)
--
--       Z = c[1]*x[m+1] + c[2]*x[m+2] + ... + c[n]*x[m+n] + c[0]    (1)
--
--    subject to linear constraints
--
--       x[1] = a[1,1]*x[m+1] + a[1,2]*x[m+1] + ... + a[1,n]*x[m+n]
--       x[2] = a[2,1]*x[m+1] + a[2,2]*x[m+1] + ... + a[2,n]*x[m+n]  (2)
--                . . . . . .
--       x[m] = a[m,1]*x[m+1] + a[m,2]*x[m+1] + ... + a[m,n]*x[m+n]
--
--    and bounds of variables
--
--         l[1] <= x[1]   <= u[1]
--         l[2] <= x[2]   <= u[2]                                    (3)
--             . . . . . .
--       l[m+n] <= x[m+n] <= u[m+n]
--
-- where:
-- x[1], ..., x[m]      - rows (auxiliary variables);
-- x[m+1], ..., x[m+n]  - columns (structural variables);
-- Z                    - objective function;
-- c[1], ..., c[n]      - coefficients of the objective function;
-- c[0]                 - constant term of the objective function;
-- a[1,1], ..., a[m,n]  - constraint coefficients;
-- l[1], ..., l[m+n]    - lower bounds of variables;
-- u[1], ..., u[m+n]    - upper bounds of variables.
--
-- Using vector-matrix notations the LP problem (1)-(3) can be written
-- as follows:
--
--    minimize (or maximize)
--
--       Z = c * x + c[0]                                            (4)
--
--    subject to linear constraints
--
--       xR = A * xS                                                 (5)
--
--    and bounds of variables
--
--       l <= x <= u                                                 (6)
--
-- where:
-- xR                   - vector of auxiliary variables;
-- xS                   - vector of structural variables;
-- x = (xR, xS)         - vector of all variables;
-- c                    - vector of objective coefficients;
-- A                    - constraint matrix (has m rows and n columns);
-- l                    - vector of lower bounds of variables;
-- u                    - vector of upper bounds of variables.
--
-- The system of constraints (5) can be written in homogeneous form as
-- follows:
--
--    A~ * x = 0,                                                    (7)
--
-- where
--
--    A~ = (I | -A)                                                  (8)
--
-- is an augmented constraint matrix (has m rows and m+n columns), I is
-- the unity matrix of the order m. Note that in the structure LPX only
-- the original constraint matrix A is explicitly stored.
--
-- The current basis is defined by partitioning columns of the matrix
-- A~ into basic and non-basic ones, in which case the system (7) can
-- be written as
--
--    B * xB + N * xN = 0,                                           (9)
--
-- where B is a square non-sigular mxm matrix built of basic columns
-- and called the basis matrix, N is a mxn matrix built of non-basic
-- columns, xB is vector of basic variables, xN is vector of non-basic
-- variables.
--
-- Using the partitioning (9) the LP problem (4)-(6) can be written in
-- a form, which defines components of the corresponding basic solution
-- and is called the simplex table:
--
--    Z = d * xN + c[0]                                             (10)
--
--    xB = A^ * xN                                                  (11)
--
--    lB <= xB <= uB                                                (12)
--
--    lN <= xN <= uN                                                (13)
--
-- where:
--
--    A^ = (alfa[i,j]) = - inv(B) * N                               (14)
--
-- is the mxn matrix of influence coefficients;
--
--    d = (d[j]) = cN - N' * pi                                     (15)
--
-- is the vector of reduced costs of non-basic variables; and
--
--    pi = (pi[i]) = inv(B') * cB                                   (16)
--
-- is the vector of simplex (Lagrange) multipliers, which correspond to
-- the equiality constraints (5).
--
-- Note that signs of the reduced costs d are determined by the formula
-- (15) in both cases of minimization and maximization.
--
-- The structure LPX allows scaling the problem. In the scaled problem
-- the constraint matrix is scaled and has the form:
--
--    A" = R * A * S,                                               (17)
--
-- where A is the constraint matrix of the original (unscaled) problem,
-- R and S are, respectively, diagonal scaling mxm and nxn matrices with
-- positive diagonal elements used to scale rows and columns of A.
--
-- The connection between the original and scaled components is defined
-- by (17) and expressed with the following formulae:
--
--    c"  = S * c          (objective coefficients)
--
--    xR" = R * xR         (values of auxiliary variables)
--    lR" = R * lR         (lower bounds of auxiliary variables)
--    uR" = R * uR         (upper bounds of auxiliary variables)
--
--    xS" = inv(S) * xS    (values of structural variables)
--    lS" = inv(S) * lS    (lower bounds of structural variables)
--    uS" = inv(S) * uS    (upper bounds of structural variables)
--
--    A"  = R * A * S      (constraint matrix)
--
-- Note that substitution scaled components into (4)-(6) gives the same
-- LP problem. */

#define LPX glp_prob

#ifndef _GLP_PROB
#define _GLP_PROB

typedef struct LPX LPX;
typedef struct LPXROW LPXROW;
typedef struct LPXCOL LPXCOL;
typedef struct LPXAIJ LPXAIJ;
typedef struct LPXCPS LPXCPS;

struct glp_prob
{     /* LP/MIP problem object */
      /*--------------------------------------------------------------*/
      /* memory management */
      DMP *row_pool;
      /* memory pool for LPXROW objects */
      DMP *col_pool;
      /* memory pool for LPXCOL objects */
      DMP *aij_pool;
      /* memory pool for LPXAIJ objects */
      DMP *str_pool;
      /* memory pool for segmented character strings */
      char *str_buf; /* char str_buf[255+1]; */
      /* working buffer to store character strings */
      /*--------------------------------------------------------------*/
      /* LP/MIP data */
      STR *name;
      /* problem name (1 to 255 chars); NULL means no name is assigned
         to the problem */
      STR *obj;
      /* objective function name (1 to 255 chars); NULL means no name
         is assigned to the objective function */
      int dir;
      /* optimization direction flag (objective "sense"):
         GLP_MIN - minimization
         GLP_MAX - maximization */
      double c0;
      /* constant term of the objective function ("shift") */
      int m_max;
      /* length of the array of rows (enlarged automatically) */
      int n_max;
      /* length of the array of columns (enlarged automatically) */
      int m;
      /* number of rows, 0 <= m <= m_max */
      int n;
      /* number of columns, 0 <= n <= n_max */
      int nz;
      /* number of non-zero constraint coefficients */
      LPXROW **row; /* LPXROW *row[1+m_max]; */
      /* row[0] is not used;
         row[i], 1 <= i <= m, is a pointer to i-th row */
      LPXCOL **col; /* LPXCOL *col[1+n_max]; */
      /* col[0] is not used;
         col[j], 1 <= j <= n, is a pointer to j-th column */
      AVLTREE *r_tree;
      /* row index to find rows by their names; NULL means this index
         does not exist */
      AVLTREE *c_tree;
      /* column index to find columns by their names; NULL means this
         index does not exist */
      /*--------------------------------------------------------------*/
      /* LP basis matrix */
      BFI *b_inv; /* BFI b_inv[1:m,1:m]; */
      /* factorization (invertable form) of the current basis matrix */
      int *basis; /* int basis[1+m_max]; */
      /* basis header (valid only if the factorization is valid):
         basis[0] is not used;
         basis[i] = k is the ordinal number of auxiliary (1 <= k <= m)
         or structural (m+1 <= k <= m+n) variable which corresponds to
         i-th basic variable xB[i], 1 <= i <= m */
      /*--------------------------------------------------------------*/
      /* LP/MIP solution */
      int p_stat;
      /* status of primal basic solution: */
#if 0
#define LPX_P_UNDEF     132   /* primal solution is undefined */
#define LPX_P_FEAS      133   /* solution is primal feasible */
#define LPX_P_INFEAS    134   /* solution is primal infeasible */
#define LPX_P_NOFEAS    135   /* no primal feasible solution exists */
#endif
      int d_stat;
      /* status of dual basic solution: */
#if 0
#define LPX_D_UNDEF     136   /* dual solution is undefined */
#define LPX_D_FEAS      137   /* solution is dual feasible */
#define LPX_D_INFEAS    138   /* solution is dual infeasible */
#define LPX_D_NOFEAS    139   /* no dual feasible solution exists */
#endif
      int some;
      /* ordinal number of some auxiliary or structural variable which
         has certain property, 0 <= some <= m+n */
      double b_obj;
      /* objective function value (basic solution) */
      int t_stat;
      /* status of interior-point solution: */
#if 0
#define LPX_T_UNDEF     150   /* interior solution is undefined */
#define LPX_T_OPT       151   /* interior solution is optimal */
#endif
      double t_obj;
      /* objective function value (interior-point solution) */
      int i_stat;
      /* status of integer solution: */
#if 0
#define LPX_I_UNDEF     170   /* integer solution is undefined */
#define LPX_I_OPT       171   /* integer solution is optimal */
#define LPX_I_FEAS      172   /* integer solution is feasible */
#define LPX_I_NOFEAS    173   /* no integer solution exists */
#endif
      double i_obj;
      /* objective function value (integer solution) */
      /*--------------------------------------------------------------*/
      LPXCPS *cps;
      /* control parameters and statistics */
};

struct LPXROW
{     /* LP/MIP row (auxiliary variable) */
      int i;
      /* ordinal number (1 to m) assigned to this row */
      STR *name;
      /* row name (1 to 255 chars); NULL means no name is assigned to
         this row */
      AVLNODE *node;
      /* pointer to corresponding node in the row index; NULL means
         that either the row index does not exist or this row has no
         name assigned */
      int type;
      /* type of the auxiliary variable:
         GLP_FR - free variable
         GLP_LO - variable with lower bound
         GLP_UP - variable with upper bound
         GLP_DB - double-bounded variable
         GLP_FX - fixed variable */
      double lb; /* non-scaled */
      /* lower bound; if the row has no lower bound, lb is zero */
      double ub; /* non-scaled */
      /* upper bound; if the row has no upper bound, ub is zero */
      /* if the row type is GLP_FX, ub is equal to lb */
      LPXAIJ *ptr; /* non-scaled */
      /* pointer to doubly linked list of constraint coefficients which
         are placed in this row */
      double rii;
      /* diagonal element r[i,i] of the scaling matrix R (see (17)) for
         this row; if the scaling is not used, r[i,i] is 1 */
      int stat;
      /* status of the auxiliary variable:
         GLP_BS - basic variable
         GLP_NL - non-basic variable on lower bound
         GLP_NU - non-basic variable on upper bound
         GLP_NF - non-basic free variable
         GLP_NS - non-basic fixed variable */
      int b_ind;
      /* if the auxiliary variable is basic (LPX_BS), lpx.basis[b_ind]
         refers to this row; if the auxiliary variable is non-basic,
         b_ind is 0; this attribute is valid only if the factorization
         of the basis matrix is valid */
      double prim; /* non-scaled */
      /* primal value of the auxiliary variable in basic solution */
      double dual; /* non-scaled */
      /* dual value of the auxiliary variable in basic solution */
      double pval; /* non-scaled */
      /* primal value of the auxiliary variable in interior solution */
      double dval; /* non-scaled */
      /* dual value of the auxiliary variable in interior solution */
      double mipx; /* non-scaled */
      /* primal value of the auxiliary variable in integer solution */
};

struct LPXCOL
{     /* LP/MIP column (structural variable) */
      int j;
      /* ordinal number (1 to n) assigned to this column */
      STR *name;
      /* column name (1 to 255 chars); NULL means no name is assigned
         to this column */
      AVLNODE *node;
      /* pointer to corresponding node in the column index; NULL means
         that either the column index does not exist or the column has
         no name assigned */
      int kind;
      /* kind of the structural variable:
         GLP_CV - continuous variable
         GLP_IV - integer or binary variable */
      int type;
      /* type of the structural variable:
         GLP_FR - free variable
         GLP_LO - variable with lower bound
         GLP_UP - variable with upper bound
         GLP_DB - double-bounded variable
         GLP_FX - fixed variable */
      double lb; /* non-scaled */
      /* lower bound; if the column has no lower bound, lb is zero */
      double ub; /* non-scaled */
      /* upper bound; if the column has no upper bound, ub is zero */
      /* if the column type is GLP_FX, ub is equal to lb */
      double coef; /* non-scaled */
      /* objective coefficient at the structural variable */
      LPXAIJ *ptr; /* non-scaled */
      /* pointer to doubly linked list of constraint coefficients which
         are placed in this column */
      double sjj;
      /* diagonal element s[j,j] of the scaling matrix S (see (17)) for
         this column; if the scaling is not used, s[j,j] is 1 */
      int stat;
      /* status of the structural variable:
         GLP_BS - basic variable
         GLP_NL - non-basic variable on lower bound
         GLP_NU - non-basic variable on upper bound
         GLP_NF - non-basic free variable
         GLP_NS - non-basic fixed variable */
      int b_ind;
      /* if the structural variable is basic (LPX_BS), lpx.basis[b_ind]
         refers to this column; if the structural variable is non-basic,
         b_ind is 0; this attribute is valid only if the factorization
         of the basis matrix is valid */
      double prim; /* non-scaled */
      /* primal value of the structural variable in basic solution */
      double dual; /* non-scaled */
      /* dual value of the structural variable in basic solution */
      double pval; /* non-scaled */
      /* primal value of the structural variable in interior solution */
      double dval; /* non-scaled */
      /* dual value of the structural variable in interior solution */
      double mipx;
      /* primal value of the structural variable in integer solution */
};

struct LPXAIJ
{     /* constraint coefficient a[i,j]; see (2) and (5) */
      LPXROW *row;
      /* pointer to row, where this coefficient is placed */
      LPXCOL *col;
      /* pointer to column, where this coefficient is placed */
      double val;
      /* numeric (non-zero) value of this coefficient */
      LPXAIJ *r_prev;
      /* pointer to previous coefficient in the same row */
      LPXAIJ *r_next;
      /* pointer to next coefficient in the same row */
      LPXAIJ *c_prev;
      /* pointer to previous coefficient in the same column */
      LPXAIJ *c_next;
      /* pointer to next coefficient in the same column */
};

struct LPXCPS
{     /* control parameters and statistics */
      int msg_lev;
      /* level of messages output by the solver:
         0 - no output
         1 - error messages only
         2 - normal output
         3 - full output (includes informational messages) */
      int scale;
      /* scaling option:
         0 - no scaling
         1 - equilibration scaling
         2 - geometric mean scaling
         3 - geometric mean scaling, then equilibration scaling */
      int dual;
      /* dual simplex option:
         0 - do not use the dual simplex
         1 - if the initial basic solution being primal infeasible is
             dual feasible, use the dual simplex */
      int price;
      /* pricing option (for both primal and dual simplex):
         0 - textbook pricing
         1 - steepest edge pricing */
      double relax;
      /* relaxation parameter used in the ratio test; if it is zero,
         the textbook ratio test is used; if it is non-zero (should be
         positive), Harris' two-pass ratio test is used; in the latter
         case on the first pass basic variables (in the case of primal
         simplex) or reduced costs of non-basic variables (in the case
         of dual simplex) are allowed to slightly violate their bounds,
         but not more than (relax * tol_bnd) or (relax * tol_dj) (thus,
         relax is a percentage of tol_bnd or tol_dj) */
      double tol_bnd;
      /* relative tolerance used to check if the current basic solution
         is primal feasible */
      double tol_dj;
      /* absolute tolerance used to check if the current basic solution
         is dual feasible */
      double tol_piv;
      /* relative tolerance used to choose eligible pivotal elements of
         the simplex table in the ratio test */
      int round;
      /* solution rounding option:
         0 - report all computed values and reduced costs "as is"
         1 - if possible (allowed by the tolerances), replace computed
             values and reduced costs which are close to zero by exact
             zeros */
      double obj_ll;
      /* lower limit of the objective function; if on the phase II the
         objective function reaches this limit and continues decreasing,
         the solver stops the search */
      double obj_ul;
      /* upper limit of the objective function; if on the phase II the
         objective function reaches this limit and continues increasing,
         the solver stops the search */
      int it_lim;
      /* simplex iterations limit; if this value is positive, it is
         decreased by one each time when one simplex iteration has been
         performed, and reaching zero value signals the solver to stop
         the search; negative value means no iterations limit */
      int it_cnt;
      /* simplex iterations count; this count is increased by one each
         time when one simplex iteration has been performed */
      double tm_lim;
      /* searching time limit, in seconds; if this value is positive,
         it is decreased each time when one simplex iteration has been
         performed by the amount of time spent for the iteration, and
         reaching zero value signals the solver to stop the search;
         negative value means no time limit */
      int out_frq;
      /* output frequency, in iterations; this parameter specifies how
         frequently the solver sends information about the solution to
         the standard output */
      double out_dly;
      /* output delay, in seconds; this parameter specifies how long
         the solver should delay sending information about the solution
         to the standard output; zero value means no delay */
      int branch; /* MIP */
      /* branching heuristic:
         0 - branch on first variable
         1 - branch on last variable
         2 - branch using heuristic by Driebeck and Tomlin
         3 - branch on most fractional variable */
      int btrack; /* MIP */
      /* backtracking heuristic:
         0 - select most recent node (depth first search)
         1 - select earliest node (breadth first search)
         2 - select node using the best projection heuristic
         3 - select node with best local bound */
      double tol_int; /* MIP */
      /* absolute tolerance used to check if the current basic solution
         is integer feasible */
      double tol_obj; /* MIP */
      /* relative tolerance used to check if the value of the objective
         function is not better than in the best known integer feasible
         solution */
      int mps_info; /* lpx_write_mps */
      /* if this flag is set, the routine lpx_write_mps outputs several
         comment cards that contains some information about the problem;
         otherwise the routine outputs no comment cards */
      int mps_obj; /* lpx_write_mps */
      /* this parameter tells the routine lpx_write_mps how to output
         the objective function row:
         0 - never output objective function row
         1 - always output objective function row
         2 - output objective function row if and only if the problem
             has no free rows */
      int mps_orig; /* lpx_write_mps */
      /* if this flag is set, the routine lpx_write_mps uses original
         row and column symbolic names; otherwise the routine generates
         plain names using ordinal numbers of rows and columns */
      int mps_wide; /* lpx_write_mps */
      /* if this flag is set, the routine lpx_write_mps uses all data
         fields; otherwise the routine keeps fields 5 and 6 empty */
      int mps_free; /* lpx_write_mps */
      /* if this flag is set, the routine lpx_write_mps omits column
         and vector names everytime if possible (free style); otherwise
         the routine never omits these names (pedantic style) */
      int mps_skip; /* lpx_write_mps */
      /* if this flag is set, the routine lpx_write_mps skips empty
         columns (i.e. which has no constraint coefficients); otherwise
         the routine outputs all columns */
      int lpt_orig; /* lpx_write_lpt */
      /* if this flag is set, the routine lpx_write_lpt uses original
         row and column symbolic names; otherwise the routine generates
         plain names using ordinal numbers of rows and columns */
      int presol; /* lpx_simplex */
      /* LP presolver option:
         0 - do not use LP presolver
         1 - use LP presolver */
      int binarize; /* lpx_intopt */
      /* if this flag is set, the routine lpx_intopt replaces integer
         columns by binary ones */
      int use_cuts; /* lpx_intopt */
      /* if this flag is set, the routine lpx_intopt tries generating
         cutting planes: */
#if 0
#define LPX_C_COVER     0x01  /* mixed cover cuts */
#define LPX_C_CLIQUE    0x02  /* clique cuts */
#define LPX_C_GOMORY    0x04  /* Gomory's mixed integer cuts */
#define LPX_C_ALL       0xFF  /* all cuts */
#endif
};

#endif

/**********************************************************************/

/* basis status: */
#define LPX_B_UNDEF     130   /* current basis is undefined */
#define LPX_B_VALID     131   /* current basis is valid */

#define lpx_order_matrix      _glp_lpx_order_matrix
void lpx_order_matrix(LPX *lp);
/* order rows and columns of the constraint matrix */

#define lpx_set_rii           _glp_lpx_set_rii
void lpx_set_rii(LPX *lp, int i, double rii);
/* set (change) row scale factor */

#define lpx_set_sjj           _glp_lpx_set_sjj
void lpx_set_sjj(LPX *lp, int j, double sjj);
/* set (change) column scale factor */

#define lpx_put_lp_basis      _glp_lpx_put_lp_basis
void lpx_put_lp_basis(LPX *lp, int b_stat, int basis[], BFI *b_inv);
/* store LP basis information */

#define lpx_put_solution      _glp_lpx_put_solution
void lpx_put_solution(LPX *lp, int p_stat, int d_stat,
      int row_stat[], double row_prim[], double row_dual[],
      int col_stat[], double col_prim[], double col_dual[]);
/* store basic solution components */

#define lpx_put_ray_info      _glp_lpx_put_ray_info
void lpx_put_ray_info(LPX *lp, int k);
/* store row/column which causes unboundness */

#define lpx_put_ipt_soln      _glp_lpx_put_ipt_soln
void lpx_put_ipt_soln(LPX *lp, int t_stat, double row_pval[],
      double row_dval[], double col_pval[], double col_dval[]);
/* store interior-point solution components */

#define lpx_put_mip_soln      _glp_lpx_put_mip_soln
void lpx_put_mip_soln(LPX *lp, int i_stat, double row_mipx[],
      double col_mipx[]);
/* store mixed integer solution components */

#define lpx_get_rii           _glp_lpx_get_rii
double lpx_get_rii(LPX *lp, int i);
/* retrieve row scale factor */

#define lpx_get_sjj           _glp_lpx_get_sjj
double lpx_get_sjj(LPX *lp, int j);
/* retrieve column scale factor */

#define lpx_is_b_avail        _glp_lpx_is_b_avail
int lpx_is_b_avail(LPX *lp);
/* check if LP basis is available */

#define lpx_get_b_info        _glp_lpx_get_b_info
int lpx_get_b_info(LPX *lp, int i);
/* retrieve LP basis information */

#define lpx_get_row_b_ind     _glp_lpx_get_row_b_ind
int lpx_get_row_b_ind(LPX *lp, int i);
/* retrieve row index in LP basis */

#define lpx_get_col_b_ind     _glp_lpx_get_col_b_ind
int lpx_get_col_b_ind(LPX *lp, int j);
/* retrieve column index in LP basis */

#define lpx_access_inv        _glp_lpx_access_inv
BFI *lpx_access_inv(LPX *lp);
/* access factorization of basis matrix */

#define lpx_invert            _glp_lpx_invert
int lpx_invert(LPX *lp);
/* compute factorization of basis matrix */

#define lpx_ftran             _glp_lpx_ftran
void lpx_ftran(LPX *lp, double x[]);
/* forward transformation (solve system B*x = b) */

#define lpx_btran             _glp_lpx_btran
void lpx_btran(LPX *lp, double x[]);
/* backward transformation (solve system B'*x = b) */

#define lpx_eval_b_prim       _glp_lpx_eval_b_prim
void lpx_eval_b_prim(LPX *lp, double row_prim[], double col_prim[]);
/* compute primal basic solution components */

#define lpx_eval_b_dual       _glp_lpx_eval_b_dual
void lpx_eval_b_dual(LPX *lp, double row_dual[], double col_dual[]);
/* compute dual basic solution components */

#define lpx_remove_tiny       _glp_lpx_remove_tiny
int lpx_remove_tiny(int ne, int ia[], int ja[], double ar[],
      double eps);
/* remove zero and tiny elements */

#define lpx_reduce_form       _glp_lpx_reduce_form
int lpx_reduce_form(LPX *lp, int len, int ind[], double val[],
      double work[]);
/* reduce linear form */

#define lpx_eval_row          _glp_lpx_eval_row
double lpx_eval_row(LPX *lp, int len, int ind[], double val[]);
/* compute explicitly specified row */

#define lpx_eval_degrad       _glp_lpx_eval_degrad
double lpx_eval_degrad(LPX *lp, int len, int ind[], double val[],
      int type, double rhs);
/* compute degradation of the objective function */

#define lpx_gomory_cut        _glp_lpx_gomory_cut
int lpx_gomory_cut(LPX *lp, int len, int ind[], double val[],
      double work[]);
/* generate Gomory's mixed integer cut */

#define lpx_cover_cut         _glp_lpx_cover_cut
int lpx_cover_cut(LPX *lp, int len, int ind[], double val[],
      double work[]);
/* generate mixed cover cut */

#define lpx_create_cog        _glp_lpx_create_cog
void *lpx_create_cog(LPX *lp);
/* create the conflict graph */

#define lpx_add_cog_edge      _glp_lpx_add_cog_edge
void lpx_add_cog_edge(void *cog, int i, int j);
/* add edge to the conflict graph */

#define lpx_clique_cut        _glp_lpx_clique_cut
int lpx_clique_cut(LPX *lp, void *cog, int ind[], double val[]);
/* generate clique cut */

#define lpx_delete_cog        _glp_lpx_delete_cog
void lpx_delete_cog(void *cog);
/* delete the conflict graph */

#define lpx_extract_prob      _glp_lpx_extract_prob
LPX *lpx_extract_prob(void *mpl);
/* extract problem instance from MathProg model */

#define lpx_read_prob         _glp_lpx_read_prob
LPX *lpx_read_prob(char *fname);
/* read problem data in GNU LP format */

#define lpx_write_prob        _glp_lpx_write_prob
int lpx_write_prob(LPX *lp, char *fname);
/* write problem data in GNU LP format */

#endif

/* eof */
