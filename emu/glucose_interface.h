#pragma once
#include "bprolog.h"


#ifdef __cplusplus
extern "C" {
#endif
	void glu_init();
	void glu_add_lit(int lit0);
	int glu_start_solver();
	int glu_get_binding(int varNum);


	void pglu_init();
	void pglu_add_lit(int lit0);
	int pglu_start_solver();
	int pglu_get_binding(int varNum);
#ifdef __cplusplus
}
#endif