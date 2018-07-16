#include "bprolog.h"
#ifdef SAT
/********************************************************************
 *   File   : espresso_bp.c
 *   Author : Neng-Fa ZHOU Copyright (C) 1994-2017
 *   Purpose: Interface with Espresso for Picat

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. 
 ********************************************************************/

#include "espresso.h"

void prep_espresso();
void run_espresso(pPLA PLA);
void stop_espresso(pPLA PLA);
pPLA init_PLA(int n);
void setup_PLA(BPLONG Vals, BPLONG InFlag, pPLA PLA);
void setup_PLA_element(BPLONG V_K, BPLONG ElmsVect, pPLA PLA);
void setup_PLA_table(BPLONG InFlag, BPLONG VarsVect, BPLONG K, BPLONG HTable, pPLA PLA);
void retrieve_pla_cubes(BPLONG_PTR ptrBNs, pPLA PLA, BPLONG Cls, BPLONG ClsR);
void setup_PLA_pb(BPLONG Coes, BPLONG pb_rel, BPLONG pb_const, pPLA PLA);

/************************************************************************************************** 
   call_espresso(BNs,InFlag,Vals,Cls,ClsR):

   BNs      : A vector of Boolean variable numbers, v(BN0,BN1,...,BN(n-1)), where BNi is ignored if f.
   Vals     : A sorted list of values or intervals. All the values must be non-negative.
   InFlag   : 1 or 0, indicating if it's a 'X :: D' or 'X notin D' domain constraint.
   Cls-ClsR : A list of clauses to be returned.

   Let V be the variable encoded by BNs (log-encoding), Vals be {a1,a2,...,an}. If InFlag=1, then Cls encodes 

                        V = a1 \/ V = a2 \/ ... \/ V = an.

   If InFlag=0, then Cls encodes

                        V != a1 /\ V != a2 /\ ... /\ V != an.
***************************************************************************************************/
int c_call_espresso()
{
    pPLA PLA;
    BPLONG n;
    BPLONG BNVect,Vals,InFlag,Cls,ClsR;
    BPLONG_PTR ptrBNVect;

    SYM_REC_PTR sym_ptr;

    prep_espresso();

    BNVect = ARG(1,5); DEREF_NONVAR(BNVect);
    Vals = ARG(2,5); DEREF_NONVAR(Vals);
    InFlag = ARG(3,5); DEREF_NONVAR(InFlag);
    Cls = ARG(4,5);
    ClsR = ARG(5,5);

    ptrBNVect = (BPLONG_PTR)UNTAGGED_ADDR(BNVect);
    sym_ptr = (SYM_REC_PTR)FOLLOW(ptrBNVect);
    n = GET_ARITY(sym_ptr);  /* n bits (Boolean variables) */
    PLA = init_PLA(n);

    //  write_term(Vals);

    setup_PLA(Vals,INTVAL(InFlag),PLA);
    //  fprint_pla(curr_out,PLA, FD_type);

    run_espresso(PLA);

    /* Output the solution */
    retrieve_pla_cubes(ptrBNVect,PLA,Cls,ClsR);
    //  EXECUTE(fprint_pla(stdout, PLA, F_type), WRITE_TIME, PLA->F, cost);

    stop_espresso(PLA);
    return BP_TRUE;
}

/* initialize global variables used in espresso */
void prep_espresso(){
#ifdef RANDOM
    srandom(314973);
#endif
    debug = 0;                      /* default -d: no debugging info */
    verbose_debug = FALSE;          /* default -v: not verbose */
    print_solution = FALSE;         /* default -x: print the solution (!) */
    summary = FALSE;                /* default -s: no summary */
    trace = FALSE;                  /* default -t: no trace information */
    remove_essential = TRUE;        /* default -e: */
    force_irredundant = TRUE;
    unwrap_onset = TRUE;
    single_expand = FALSE;
    pos = FALSE;
    recompute_onset = FALSE;
    use_super_gasp = FALSE;
    use_random_order = FALSE;
    kiss = FALSE;
    echo_comments = TRUE;
    echo_unknown_commands = TRUE;
}

void run_espresso(pPLA PLA){
    pcover F, Fold, Dold;
    bool error, exact_cover;
    cost_t cost;

    exact_cover = FALSE;        /* for -qm option, the default */
    Fold = sf_save(PLA->F);
    PLA->F = espresso(PLA->F, PLA->D, PLA->R);
    EXECUTE(error=verify(PLA->F,Fold,PLA->D), VERIFY_TIME, PLA->F, cost);
    if (error) {
        print_solution = FALSE;
        PLA->F = Fold;
        (void) check_consistency(PLA);
    } else {
        free_cover(Fold);
    }
    if (error) {
        fatal("cover verification failed");
    }
}

void stop_espresso(pPLA PLA){
    /* Crash and burn if there was a verify error */

    /* cleanup all used memory */
    free_PLA(PLA);
    FREE(cube.part_size);
    setdown_cube();             /* free the cube/cdata structure data */
    sf_cleanup();               /* free unused set structures */
    sm_cleanup();               /* sparse matrix cleanup */
}

pPLA init_PLA(int n){
    pPLA PLA;

    PLA = new_PLA();
    PLA->pla_type = FD_type;

    cube.num_binary_vars = n;
    cube.num_vars = cube.num_binary_vars + 1;
    cube.part_size = ALLOC(int, cube.num_vars);

    cube.part_size[cube.num_vars-1] = 1;
    cube_setup();

    PLA_labels(PLA);

    if (PLA->F == NULL) {
        PLA->F = new_cover(10);
        PLA->D = new_cover(10);
        PLA->R = new_cover(10);
    }
  
    return PLA;
}

/* 
   Add each domain value as a cube into PLA. Since CNF is needed, 
   the output of an in-value is 0, and the output of an out-value is 1.
*/
void setup_PLA(BPLONG Vals, BPLONG InFlag, pPLA PLA){
    int needs_dcset, needs_offset;
    register int var, i;
    pcube cf, cr, cd;
    bool savef, saved, saver;
    int val, val_is_in, tmp_val;

    needs_dcset = 1;
    needs_offset = 1;

    //  printf("read_cubes n=%d 2^n=%d\n",cube.num_binary_vars,2<<(cube.num_binary_vars));

    for (val = 0; val < 1<<(cube.num_binary_vars); val++){  /* NOTE: cube is a global variable defined in espresso.h */
        if (PLA->F == NULL) {
            PLA->F = new_cover(10);
            PLA->D = new_cover(10);
            PLA->R = new_cover(10);
        }
        savef = FALSE, saved = FALSE, saver = FALSE;
        cf = cube.temp[0], cr = cube.temp[1], cd = cube.temp[2];
        set_clear(cf, cube.size);
        
        tmp_val = val;
        for(var = 0; var < cube.num_binary_vars; var++){
            if (tmp_val%2 == 1){
                set_insert(cf, var*2+1);
            } else {
                set_insert(cf, var*2);
            }
            tmp_val = tmp_val/2;
        }
        set_copy(cr, cf);
        set_copy(cd, cf);

        //      printf("cube.first_part[var]=%d cube.last_part[var]=%d\n",cube.first_part[var], cube.last_part[var]);

        i = cube.first_part[var];

        if (ISLIST(Vals)){
            BPLONG_PTR lst_ptr;
            BPLONG elm;

            lst_ptr = (BPLONG_PTR)UNTAGGED_ADDR(Vals);
            elm = FOLLOW(lst_ptr);
            DEREF_NONVAR(elm);
            if (ISINT(elm)){
                elm = INTVAL(elm);
                if (val == elm){
                    val_is_in = InFlag;
                    Vals = FOLLOW(lst_ptr+1); DEREF_NONVAR(Vals);
                } else {
                    val_is_in = 1-InFlag;  /* 1 becomes 0, and 0 becomes 1 */
                }
            } else {  /* elm must be an interval l..u */
                BPLONG_PTR interv_ptr;
                BPLONG lb,ub;

                interv_ptr = (BPLONG_PTR)UNTAGGED_ADDR(elm);
                lb = FOLLOW(interv_ptr+1); DEREF_NONVAR(lb); lb = INTVAL(lb);
                ub = FOLLOW(interv_ptr+2); DEREF_NONVAR(ub); ub = INTVAL(ub);
                if (val >= lb && val <= ub){
                    val_is_in = InFlag;
                } else {
                    val_is_in = 1-InFlag;                 
                }
                if (val == ub){
                    Vals = FOLLOW(lst_ptr+1); DEREF_NONVAR(Vals);
                }
            }
        } else {
            val_is_in = 1-InFlag;
        }

        //      printf("val=%d flag=%d\n",val,val_is_in);
        if (val_is_in){
            if (PLA->pla_type & R_type){
                set_insert(cr, i); saver = TRUE;
            }
        } else {
            if (PLA->pla_type & F_type){
                set_insert(cf, i); savef = TRUE;
            }
        }
        
        if (savef) PLA->F = sf_addset(PLA->F, cf);
        if (saved) PLA->D = sf_addset(PLA->D, cd);
        if (saver) PLA->R = sf_addset(PLA->R, cr);
    }
    //  fprint_pla(curr_out,PLA, FD_type);

    after_setup_pla(needs_dcset, needs_offset, PLA);
}

BPLONG retrieve_pla_cube(BPLONG_PTR ptrBNVect, register pset c)
{
    register int i, var;
    BPLONG_PTR tail_ptr, heap_top0;
    BPLONG lst,elm;

    heap_top0 = heap_top;
    tail_ptr = &lst;

    for(var = 0; var < cube.num_binary_vars; var++) {
        int lit = GETINPUT(c, var);
        elm = FOLLOW(ptrBNVect+var+1);
        DEREF_NONVAR(elm);
        if (ISINT(elm)) {             
            if (lit == 1 || lit == 2){  /* 3 means don't care */
                if (lit == 2) {
                    elm = INTVAL(elm);
                    elm = MAKEINT(-elm);
                }
                FOLLOW(tail_ptr) = ADDTAG(heap_top,LST);
                FOLLOW(heap_top++) = elm;
                tail_ptr = heap_top++;
            }
        } else {                      /* elm can be 'f' or 't' */
            if ((lit == 1 && elm == t_atom) || (lit == 2 && elm == f_atom)){
                heap_top = heap_top0;
                return BP_TRUE;           /* this clause is already true because the literal is true */
            }                             /* else this literal is false, and need not be added */
        }                             
    }
    FOLLOW(tail_ptr) = nil_sym;
    return lst;
}

void retrieve_pla_cubes(BPLONG_PTR ptrBNVect, pPLA PLA, BPLONG Cls, BPLONG ClsR){
    int num;
    register pcube last, p;
    BPLONG_PTR tail_ptr;
    BPLONG lst;
  
    tail_ptr = &lst;
    foreach_set(PLA->F, last, p) {
        BPLONG cell = retrieve_pla_cube(ptrBNVect, p);
        if (cell != BP_TRUE) {  /* if cell = BP_TRUE, then the clause is already tree */
            FOLLOW(tail_ptr) = ADDTAG(heap_top,LST);
            FOLLOW(heap_top++) = cell;
            tail_ptr = heap_top++;
            LOCAL_OVERFLOW_CHECK("espresso");
        }
    }
    if (tail_ptr == &lst){ /* empty, false */
        unify(Cls,ClsR);
    } else {
        FOLLOW(tail_ptr) = (BPLONG)tail_ptr;
        unify(ClsR,(BPLONG)tail_ptr);
        unify(Cls,lst);
    }
}

/*
  c_call_espresso_element(IV_K,ElmsVect,BNVect,Cls,ClsR).    
  Use Espresso to find a CNF for the global constraint element(IV,ElmsVect,V).
  IV_K     : the number of Boolean variables used for IV.
  BNVect : The Boolean variable numbers used for IV and V
*/
int c_call_espresso_element()
{
    pPLA PLA;
    BPLONG n;
    BPLONG ElmsVect,IV_K,BNVect,Cls,ClsR;
    BPLONG_PTR ptrBNVect;
    SYM_REC_PTR sym_ptr;

    prep_espresso();
  
    IV_K = ARG(1,5); DEREF_NONVAR(IV_K);
    ElmsVect = ARG(2,5); DEREF_NONVAR(ElmsVect);
    BNVect = ARG(3,5); DEREF_NONVAR(BNVect);
    Cls = ARG(4,5);
    ClsR = ARG(5,5);

    ptrBNVect = (BPLONG_PTR)UNTAGGED_ADDR(BNVect);
    sym_ptr = (SYM_REC_PTR)FOLLOW(ptrBNVect);
    n = GET_ARITY(sym_ptr);  /* n bits (Boolean variables) */
    PLA = init_PLA(n);

    //  printf("=>element n = %d IV_K=%d\n",n, INTVAL(IV_K)); // write_term(BNVect); write_term(ElmsVect); printf("\n");

  
    setup_PLA_element(INTVAL(IV_K),ElmsVect,PLA);
    //  fprint_pla(curr_out,PLA, FD_type);

    run_espresso(PLA);

    /* Output the solution */
    retrieve_pla_cubes(ptrBNVect,PLA,Cls,ClsR);
    //  printf("<=element "); write_term(Cls); printf("\n");
    //  EXECUTE(fprint_pla(stdout, PLA, F_type), WRITE_TIME, PLA->F, cost);

    stop_espresso(PLA);
    return BP_TRUE;
}

/* 
   For each value in 0..2**n-1, the lowest IV_K bits represent IV, and the remaining bits represent V.
   If element(IV,ElmsVect,V) is true, then the cube is 0; otherwise, the cube is 1 (Note it's turned 
   upside down because CNF is computed, not DNF).
*/
void setup_PLA_element(BPLONG IV_K, BPLONG ElmsVect, pPLA PLA){
    int needs_dcset, needs_offset;
    register int var, i;
    pcube cf, cr, cd;
    bool savef, saved, saver;
    int val, val_is_in, tmp_val;
    SYM_REC_PTR sym_ptr;
    BPLONG_PTR ptrElmsVect,ptrElmsOnStack;
    int nElms,iv,v;

    /* untag the elements and copy them to the stack */
    ptrElmsVect = (BPLONG_PTR)UNTAGGED_ADDR(ElmsVect);
    sym_ptr = (SYM_REC_PTR)FOLLOW(ptrElmsVect);
    nElms = GET_ARITY(sym_ptr);  /* Number of elements in element(IV,ElmsVect,V) */
  
    ptrElmsOnStack = local_top-1;
    if (local_top-heap_top-nElms <= SMALL_MARGIN){
        myquit(STACK_OVERFLOW,"espresso");
    }
    for (i = 1; i <= nElms; i++){
        BPLONG elm;
        elm = FOLLOW(ptrElmsVect+i); DEREF_NONVAR(elm); 
        FOLLOW(ptrElmsOnStack-i) = INTVAL(elm);
    }
        
    needs_dcset = 1;
    needs_offset = 1;
  
    //  printf("read_cubes n=%d 2^n=%d\n",cube.num_binary_vars,2<<(cube.num_binary_vars));

    for (val = 0; val < 1<<(cube.num_binary_vars); val++){  /* NOTE: cube is a global variable defined in espresso.h */
        if (PLA->F == NULL) {
            PLA->F = new_cover(10);
            PLA->D = new_cover(10);
            PLA->R = new_cover(10);
        }
        savef = FALSE, saved = FALSE, saver = FALSE;
        cf = cube.temp[0], cr = cube.temp[1], cd = cube.temp[2];
        set_clear(cf, cube.size);
        
        tmp_val = val;
        for(var = 0; var < cube.num_binary_vars; var++){
            if (tmp_val%2 == 1){
                set_insert(cf, var*2+1);
            } else {
                set_insert(cf, var*2);
            }
            tmp_val = tmp_val/2;
        }
        set_copy(cr, cf);
        set_copy(cd, cf);

        //      printf("cube.first_part[var]=%d cube.last_part[var]=%d\n",cube.first_part[var], cube.last_part[var]);

        v = (val & (0xffffffffL << IV_K)) >> IV_K;
        iv = val - (v << IV_K);

        //      printf("val=%d iv=%d v=%d\n",val,iv,v);

        if (iv != 0 && iv <= nElms && FOLLOW(ptrElmsOnStack-iv) == v){
            //    printf("I=%d V=%d\n",iv,v);
            val_is_in = 1;
        } else {
            val_is_in = 0;
        }
        
        i = cube.first_part[var];
        //      printf("val=%d flag=%d\n",val,val_is_in);
        if (val_is_in==0){
            if (PLA->pla_type & R_type){
                set_insert(cr, i); saver = TRUE;
            }
        } else {
            if (PLA->pla_type & F_type){
                set_insert(cf, i); savef = TRUE;
            }
        }
        
        if (savef) PLA->F = sf_addset(PLA->F, cf);
        if (saved) PLA->D = sf_addset(PLA->D, cd);
        if (saver) PLA->R = sf_addset(PLA->R, cr);
    }
    //  fprint_pla(curr_out,PLA, FD_type);

    after_setup_pla(needs_dcset, needs_offset, PLA);
}

/*
  c_call_espresso_table(InFlag,VarsVect,K1,HTable,BNVect,Cls,ClsR):

  Use Espresso to find a CNF for the table constraint table_in(VarsVect,Table) or table_notin(VarsVect,Table)

  InFlag  : 1 means the constraint is table_in, and 0 means it is table_notin.
  VarsVect: {V1,V2,...,}, only V1 and V2 are dvars and all other elements are integers.
  K1      : the number of Boolean variables used for V1.
  HTable  : a hashtable built from Table
  BNVect : The Boolean variables' numbers of V1 and V2
*/
int c_call_espresso_table(){
    pPLA PLA;
    BPLONG n;
    BPLONG InFlag,VarsVect,K1,HTable,BNVect,Cls,ClsR;
    BPLONG_PTR ptrBNVect;
    SYM_REC_PTR sym_ptr;

    prep_espresso();
  
    InFlag = ARG(1,7); DEREF_NONVAR(InFlag);
    VarsVect = ARG(2,7); DEREF_NONVAR(VarsVect);
    K1 = ARG(3,7); DEREF_NONVAR(K1);
    HTable = ARG(4,7); DEREF_NONVAR(HTable);
    BNVect = ARG(5,7); DEREF_NONVAR(BNVect);
    Cls = ARG(6,7);
    ClsR = ARG(7,7);


    ptrBNVect = (BPLONG_PTR)UNTAGGED_ADDR(BNVect);
    sym_ptr = (SYM_REC_PTR)FOLLOW(ptrBNVect);
    n = GET_ARITY(sym_ptr);  /* n bits (Boolean variables) */
    PLA = init_PLA(n);

    //  printf("=>tabnle n = %d K1=%d ",n,INTVAL(K1)); write_term(VarsVect); printf("\n");
  
    setup_PLA_table(INTVAL(InFlag),VarsVect,INTVAL(K1),HTable,PLA);
    //  fprint_pla(curr_out,PLA, FD_type);

    run_espresso(PLA);

    /* Output the solution */
    retrieve_pla_cubes(ptrBNVect,PLA,Cls,ClsR);
    //  printf("<=element "); write_term(Cls); printf("\n");
    //  EXECUTE(fprint_pla(stdout, PLA, F_type), WRITE_TIME, PLA->F, cost);

    stop_espresso(PLA);
    return BP_TRUE;
}

void prepare_table_lookup(BPLONG HTable, BPLONG VarsVect, BPLONG_PTR *ptr_htable_ptr, BPLONG *ptr_htable_size, BPLONG_PTR *ptr_tuple_ptr, BPLONG *ptr_tuple_size) {
    BPLONG i,n,htable_size;
    BPLONG_PTR tuple_ptr,ptrVarsVect,htable_ptr;
    SYM_REC_PTR sym_ptr;

    DEREF_NONVAR(HTable);
    htable_ptr = (BPLONG_PTR)UNTAGGED_ADDR(HTable);
    sym_ptr = (SYM_REC_PTR)FOLLOW(htable_ptr);
    htable_size = GET_ARITY(sym_ptr);

    DEREF_NONVAR(VarsVect);
    ptrVarsVect = (BPLONG_PTR)UNTAGGED_ADDR(VarsVect);
    sym_ptr = (SYM_REC_PTR)FOLLOW(ptrVarsVect);
    n = GET_ARITY(sym_ptr);

    tuple_ptr = local_top-n-2; /* space for a tuple */

    if (tuple_ptr-heap_top <= LARGE_MARGIN){
        myquit(STACK_OVERFLOW,"espresso 12");
    }

    FOLLOW(tuple_ptr) = (BPLONG)sym_ptr; 
    for (i=3;i<=n;i++){
        BPLONG t;
        t = FOLLOW(ptrVarsVect+i);DEREF_NONVAR(t);
        FOLLOW(tuple_ptr+i) = t; 
    }
    *ptr_htable_ptr = htable_ptr;
    *ptr_htable_size = htable_size;
    *ptr_tuple_ptr = tuple_ptr;
    *ptr_tuple_size = n;
}

/* 
   For each value in 0..2**n-1, the lowest K1 bits represent V1, and the remaining bits represent V2.
   If the constraint (table_in or table_notin, depending on if InFlag=1) is true, then the cube is 0; 
   otherwise, the cube is 1 (Note it's turned upside down because CNF is computed, not DNF).
*/
void setup_PLA_table(BPLONG InFlag, BPLONG VarsVect, BPLONG K1, BPLONG HTable, pPLA PLA){
    int needs_dcset, needs_offset;
    int var, i;
    pcube cf, cr, cd;
    bool savef, saved, saver;
    int val, val_is_in, tmp_val;
    SYM_REC_PTR sym_ptr;
    BPLONG_PTR tuple_ptr, htable_ptr;
    BPLONG htable_size, tuple_size;
    BPLONG V1,V2;


    prepare_table_lookup(HTable, VarsVect, &htable_ptr, &htable_size, &tuple_ptr, &tuple_size);
    //  printf("htable_size =%d tuple_size=%d\n",htable_size,tuple_size);

    needs_dcset = 1;
    needs_offset = 1;
  
    //  printf("read_cubes n=%d 2^n=%d\n",cube.num_binary_vars,2<<(cube.num_binary_vars));

    for (val = 0; val < 1<<(cube.num_binary_vars); val++){  /* NOTE: cube is a global variable defined in espresso.h */
        if (PLA->F == NULL) {
            PLA->F = new_cover(10);
            PLA->D = new_cover(10);
            PLA->R = new_cover(10);
        }
        savef = FALSE, saved = FALSE, saver = FALSE;
        cf = cube.temp[0], cr = cube.temp[1], cd = cube.temp[2];
        set_clear(cf, cube.size);
        
        tmp_val = val;
        for(var = 0; var < cube.num_binary_vars; var++){
            if (tmp_val%2 == 1){
                set_insert(cf, var*2+1);
            } else {
                set_insert(cf, var*2);
            }
            tmp_val = tmp_val/2;
        }
        set_copy(cr, cf);
        set_copy(cd, cf);

        //      printf("cube.first_part[var]=%d cube.last_part[var]=%d\n",cube.first_part[var], cube.last_part[var]);

        V2 = (val & (0xffffffffL << K1)) >> K1;
        V1 = val - (V2 << K1);

        FOLLOW(tuple_ptr+1) = MAKEINT(V1);
        FOLLOW(tuple_ptr+2) = MAKEINT(V2);  /* fill in V1 and V2 into the tuple */
        if (htable_contains_tuple(htable_ptr,htable_size,tuple_ptr,tuple_size)){     /* defind in mic.c */
            //    printf("in_tupe "); write_term(VarsVect); printf("\n");
            val_is_in = InFlag;
        } else {
            val_is_in = 1-InFlag;
        }
        
        i = cube.first_part[var];
        //      printf("val=%d flag=%d\n",val,val_is_in);
        if (val_is_in){
            if (PLA->pla_type & R_type){
                set_insert(cr, i); saver = TRUE;
            }
        } else {
            if (PLA->pla_type & F_type){
                set_insert(cf, i); savef = TRUE;
            }
        }
        
        if (savef) PLA->F = sf_addset(PLA->F, cf);
        if (saved) PLA->D = sf_addset(PLA->D, cd);
        if (saver) PLA->R = sf_addset(PLA->R, cr);
    }
    //  fprint_pla(curr_out,PLA, FD_type);

    after_setup_pla(needs_dcset, needs_offset, PLA);
}

/*
  c_call_espresso_pb(Coes,Rel,Const,BNVect,Cls,ClsR).

  Use Espresso to find a CNF for the Pseudo Boolean constraint scalar_product(Coes,Vs) Rel Const.

  Coes    : Coefficients of the PB expression.
  Rel     : 0 (eq), 1 (ge), and 2 (neq)
  BNVect : The numbers of the Boolean variables Vs.
*/
int c_call_espresso_pb(){
    pPLA PLA;
    BPLONG n;
    BPLONG Coes,Rel,Const,BNVect,Cls,ClsR;
    BPLONG_PTR ptrBNVect;
    SYM_REC_PTR sym_ptr;

    prep_espresso();
  
    Coes = ARG(1,6); DEREF_NONVAR(Coes);
    Rel = ARG(2,6); DEREF_NONVAR(Rel);
    Const = ARG(3,6); DEREF_NONVAR(Const);
    BNVect = ARG(4,6); DEREF_NONVAR(BNVect);
    Cls = ARG(5,6);
    ClsR = ARG(6,6);

    ptrBNVect = (BPLONG_PTR)UNTAGGED_ADDR(BNVect);
    sym_ptr = (SYM_REC_PTR)FOLLOW(ptrBNVect);
    n = GET_ARITY(sym_ptr);  /* n bits (Boolean variables) */
    PLA = init_PLA(n);

    //  printf("=>espresso_pb n = %d ",n); write_term(Coes); write_term(BNVect);printf("\n");
  
    setup_PLA_pb(Coes,INTVAL(Rel),INTVAL(Const),PLA);
    //  fprint_pla(curr_out,PLA, FD_type);

    run_espresso(PLA);

    /* Output the solution */
    retrieve_pla_cubes(ptrBNVect,PLA,Cls,ClsR);
    //  printf("<=element "); write_term(Cls); printf("\n");
    //  EXECUTE(fprint_pla(stdout, PLA, F_type), WRITE_TIME, PLA->F, cost);

    stop_espresso(PLA);
    return BP_TRUE;
}

/* 
   Let Coes = [A1,A2,...,An]. For each value in 0..2**n-1, get the valuation for V1,V2,...,Vn.
   If the constraint sum(Ai*Vi) Rel Const is true, then the cube is 0; 
   otherwise, the cube is 1 (Note it's turned upside down because CNF is computed, not DNF).
*/
void setup_PLA_pb(BPLONG Coes, BPLONG pb_rel, BPLONG pb_const, pPLA PLA){
    int needs_dcset, needs_offset;
    int var, i, arg_i, pb_val;
    pcube cf, cr, cd;
    bool savef, saved, saver;
    int val, val_is_in, tmp_val;
    BPLONG_PTR coes_ptr;
    SYM_REC_PTR sym_ptr;

    needs_dcset = 1;
    needs_offset = 1;
  
    //  printf("read_cubes n=%d 2^n=%d\n",cube.num_binary_vars,2<<(cube.num_binary_vars));
    arg_i = 0;
    coes_ptr = local_top;
    while (ISLIST(Coes)){
        BPLONG_PTR ptrCoes = (BPLONG_PTR)UNTAGGED_ADDR(Coes);
        BPLONG elm = FOLLOW(ptrCoes); DEREF_NONVAR(elm);
        FOLLOW(coes_ptr-arg_i) = INTVAL(elm);
        Coes = FOLLOW(ptrCoes+1); DEREF_NONVAR(Coes);
        arg_i++;
    }

    for (val = 0; val < 1<<(cube.num_binary_vars); val++){  /* NOTE: cube is a global variable defined in espresso.h */
        if (PLA->F == NULL) {
            PLA->F = new_cover(10);
            PLA->D = new_cover(10);
            PLA->R = new_cover(10);
        }
        pb_val = 0; arg_i = 0;
        savef = FALSE, saved = FALSE, saver = FALSE;
        cf = cube.temp[0], cr = cube.temp[1], cd = cube.temp[2];
        set_clear(cf, cube.size);
        
        tmp_val = val;
        for(var = 0; var < cube.num_binary_vars; var++){
            if (tmp_val%2 == 1){
                set_insert(cf, var*2+1);
                pb_val += FOLLOW(coes_ptr-arg_i);
            } else {
                set_insert(cf, var*2);
            }
            tmp_val = tmp_val/2;
            arg_i++;
        }
        set_copy(cr, cf);
        set_copy(cd, cf);

        //      printf("cube.first_part[var]=%d cube.last_part[var]=%d\n",cube.first_part[var], cube.last_part[var]);

        val_is_in = 0;
        switch (pb_rel){
        case 0:   // eq
            if (pb_val == pb_const) val_is_in = 1;
            break;
        case 1:  // ge
            if (pb_val >= pb_const) val_is_in = 1;
            break;
        case 2:  // neq
            if (pb_val != pb_const) val_is_in = 1;
            break;
        }

        i = cube.first_part[var];
        //      printf("val=%d flag=%d\n",val,val_is_in);
        if (val_is_in){
            if (PLA->pla_type & R_type){
                set_insert(cr, i); saver = TRUE;
            }
        } else {
            if (PLA->pla_type & F_type){
                set_insert(cf, i); savef = TRUE;
            }
        }
        
        if (savef) PLA->F = sf_addset(PLA->F, cf);
        if (saved) PLA->D = sf_addset(PLA->D, cd);
        if (saver) PLA->R = sf_addset(PLA->R, cr);
    }
    //  fprint_pla(curr_out,PLA, FD_type);

    after_setup_pla(needs_dcset, needs_offset, PLA);
}
#else
int c_call_espresso(){
    BPLONG er  = ADDTAG(BP_NEW_SYM("sat_not_supported",0),ATM);

    printf("SAT not supported for MVC. Use the cygwin version.\n");
    exception = er;
    return BP_ERROR;
}

int c_call_espresso_pb(){
    BPLONG er  = ADDTAG(BP_NEW_SYM("sat_not_supported",0),ATM);

    printf("SAT not supported for MVC. Use the cygwin version.\n");
    exception = er;
    return BP_ERROR;
}

int c_call_espresso_table(){
    BPLONG er  = ADDTAG(BP_NEW_SYM("sat_not_supported",0),ATM);

    printf("SAT not supported for MVC. Use the cygwin version.\n");
    exception = er;
    return BP_ERROR;
}
  
int c_call_espresso_element(){
    BPLONG er  = ADDTAG(BP_NEW_SYM("sat_not_supported",0),ATM);

    printf("SAT not supported for MVC. Use the cygwin version.\n");
    exception = er;
    return BP_ERROR;
}
#endif
