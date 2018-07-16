/********************************************************************
 *   File   : sat_bp.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2016
 *   Purpose: SAT interface for B-Prolog

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include "bprolog.h"

#ifdef SAT
#ifdef WIN32
#include "minisat/minisat.h"
solver* mini_s;
veci mini_lits;
lit* mini_begin;
#else
#include "lingeling/lglib.h"
static LGL * bp_lgl;
#endif

#ifdef WIN32
#define SAT_INIT mini_s = solver_new(); veci_new(&mini_lits); veci_resize(&mini_lits,0)
#define SAT_ADD_LIT(lit0)  mini_add_lit(lit0)
#define SAT_START_SOLVER res = solver_solve(mini_s,0,0)
#define SAT_SATISFIABLE res == l_True
#define SAT_GET_BINDING(varNum) ((mini_s->model.ptr[varNum-1] == l_True) ? BP_ONE : BP_ZERO)
#else 
#define SAT_INIT bp_lgl = lglinit()
#define SAT_ADD_LIT(lit0) lgladd(bp_lgl,lit0)
#define SAT_START_SOLVER res = lglsat(bp_lgl)
#define SAT_SATISFIABLE res == 10
#define SAT_GET_BINDING(varNum) ((lglderef(bp_lgl, varNum) > 0) ? BP_ONE : BP_ZERO)
#endif
#endif

    static int sat_dump_flag = 0;
static int sat_count_flag = 0;
static int num_cls = 0;

int c_sat_start_dump(){
    sat_dump_flag = 1;
    return BP_TRUE;
}

int c_sat_stop_dump(){
    sat_dump_flag = 0;
    return BP_TRUE;
}

int c_sat_start_count(){
    sat_dump_flag = 1;
    sat_count_flag = 1;
    num_cls = 0;
    return BP_TRUE;
}

int c_sat_stop_count(){
    sat_dump_flag = 0;
    sat_count_flag = 0;
    unify(ARG(1,1), MAKEINT(num_cls));
    num_cls = 0;
    return BP_TRUE;
}

#ifdef SAT
#ifdef WIN32
void mini_add_lit(BPLONG lit0){
    if (lit0==0) {
        mini_begin = veci_begin(&mini_lits);
        solver_addclause(mini_s, mini_begin, mini_begin+veci_size(&mini_lits));
        veci_resize(&mini_lits,0);
    } else if (lit0>0) {
        veci_push(&mini_lits, toLit(lit0-1));
    } else {
        veci_push(&mini_lits, lit_neg(toLit(-lit0-1)));
    }
}
#endif

/* cl is a list of literals */
int b_SAT_ADD_CL_c(BPLONG cl){
    BPLONG lit;
    BPLONG cl0 = cl;

    DEREF_NONVAR(cl);
    while (ISLIST(cl)){
        BPLONG_PTR lst_ptr;
        lst_ptr = (BPLONG_PTR)UNTAGGED_ADDR(cl);
        lit = FOLLOW(lst_ptr); DEREF_NONVAR(lit); 
        cl = FOLLOW(lst_ptr+1); DEREF_NONVAR(cl);
        if (sat_dump_flag) {
            write_term(lit); 
            write_space();
        } else {
            lit = INTVAL(lit);
            SAT_ADD_LIT(lit);
        }
    }
    if (sat_dump_flag){
        num_cls++;
        write_term(BP_ZERO);
        b_NL();
    } else {
        SAT_ADD_LIT(0); 
    }
    return BP_TRUE;
}

int c_sat_init(){
    SAT_INIT;
    return BP_TRUE;
}

int c_sat_start(){
    BPLONG lst,lst0,res;
    BPLONG_PTR top;

    lst0 = lst = ARG(1,1);
    DEREF_NONVAR(lst); 

    SAT_START_SOLVER;

    if (SAT_SATISFIABLE){
        BPLONG_PTR ptr;
        BPLONG var,varNum;

        while (ISLIST(lst)){
            BPLONG_PTR sv_ptr;
            ptr = (BPLONG_PTR)UNTAGGED_ADDR(lst);
            var = FOLLOW(ptr); DEREF(var);
            if (IS_SUSP_VAR(var)){
                sv_ptr = (BPLONG_PTR)UNTAGGED_TOPON_ADDR(var);
                varNum = fast_get_attr(sv_ptr,et_NUMBER); 
                DEREF(varNum);
                varNum = INTVAL(varNum);
                unify(var,SAT_GET_BINDING(varNum));
            }
            lst = FOLLOW(ptr+1); DEREF(lst);
        }
        return BP_TRUE;
    } 
    return BP_FALSE;
}

void Cboot_sat(){
    insert_cpred("c_sat_init",0,c_sat_init);
    insert_cpred("c_sat_start",1,c_sat_start);
}
#else
int c_sat_init(){
    BPLONG er  = ADDTAG(BP_NEW_SYM("sat_not_supported",0),ATM);

    printf("SAT not supported for MVC. Use the cygwin version.\n");
    exception = er;
    return BP_ERROR;
}

int c_sat_start(){
    printf("SAT not supported for MVC. Use the cygwin version.\n");
    return BP_FALSE;
}
void Cboot_sat(){
    insert_cpred("c_sat_init",0,c_sat_init);
    insert_cpred("c_sat_start",1,c_sat_start);
}
#endif




