#ifndef GCC
 case throw_ball:  /* y,y */
#endif
 lab_throw_ball: 
    op1 = NextOperandYC;
    op2 = (BPLONG)NextOperandY;
	printf("throw_ball %x %x\n",op1,op2);
    {
	  void printTable();
        BPLONG catch_all_flag,except;
        BPLONG_PTR btm_ptr;
        BPLONG_PTR af = AR; /* ancestor frame */
        BPLONG_PTR b = B;
        while ((BPLONG)af != AR_AR(af)){
		    while (b <= af){
                RESET_SUBGOAL_AR(b);
                if (IS_CATCHER_FRAME(b)){
                    btm_ptr = (BPLONG_PTR)UNTAGGED_ADDR(AR_BTM(b)); /* a catcher frame is in the form of p(Flag,Cleanup,Calll,Exception,Recovery,...) */
                    catch_all_flag = FOLLOW(btm_ptr); 
                    except = FOLLOW(btm_ptr-3); 
                    if ((catch_all_flag==BP_ONE || b==af) && is_UNIFIABLE(op1,except)){ /* this catcher catches the exception */

					  printf("pring table \n");
					  printTable();
					  
                        FOLLOW(op2) = ADDTAG((BPULONG)stack_up_addr-(BPULONG)b,INT_TAG);
                        CONTCASE;
                    }
                }
                b = (BPLONG_PTR)AR_B(b);
            }
            af = (BPLONG_PTR)AR_AR(af);
        }
        FOLLOW(op2) = ADDTAG((BPULONG)stack_up_addr-(BPULONG)af,INT_TAG);

		
        CONTCASE;
    }
