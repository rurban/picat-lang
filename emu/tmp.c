#define NUM_PICAT_TABLE_MAPS 97
BPLONG_PTR picat_table_maps[NUM_PICAT_TABLE_MAPS];
BPLONG picat_table_map_ids[NUM_PICAT_TABLE_MAPS];

typedef struct {
  BPLONG key;
  BPLONG val;
  BPLONG_PTR next;
} KEY_VAL_PAIR;

typedef KEY_VAL_PAIR *KEY_VAL_PAIR_PTR;

typedef struct {
  BPLONG size;
  BPLONG count;
  BPLONG_PTR htable;
} MAP_RECORD;

typedef MAP_RECORD *MAP_RECORD_PTR;

void init_picat_table_maps(){
  BPLONG i;
  
  for (i = 0; i < NUM_PICAT_TABLE_MAPS; i++){
	picat_table_maps[i] = NULL;
  }
}

/* Return the number of the map with map_id. If no map with the id was found,
   then create a new map and register it into table_maps. Linear prob is used 
   to look for the map with map_id.

   Each entry in table_maps is a pointer to a MAP_RECORD, which stores the 
   information about a map, including the size of the bucket table, the number 
   of key-value pairs (count), and a pointer to the bucket table (htable).
*/
int b_GET_PICAT_TABLE_MAP_cf(BPLONG map_id, BPLONG map_num){
  BPLONG i0, i, map_id_cp, this_hcode, this_ground_flag;
  BPLONG_PTR map_ptr ;
  
  PREPARE_NUMBER_TERM(0);
  map_id_cp = numberVarCopyToTableArea(ta_record_ptr,map_id,&this_hcode,&this_ground_flag);
  if (map_id_cp == BP_ERROR) return BP_ERROR;
  if (this_ground_flag == 0){
	exception = ground_expected;
	return BP_ERROR;
  }
  i0 = i = this_hcode % NUM_PICAT_TABLE_MAPS;  
  // linear prob 
  while ((BPLONG_PTR)picat_table_maps[i] != NULL){
	if (picat_table_map_ids[i] == map_id_cp){
	  return unify(map_num, MAKEINT(i));
	}
	i++;
	if (i == NUM_PICAT_TABLE_MAPS) i = 0;
	if (i == i0) quit("TABLE MAPS OVERFLOW");
  }
  // Come here if map_id was not found. Register a map in slot i
  ALLOCATE_FROM_NUMBERED_TERM_AREA(ta_record_ptr,map_ptr,sizeof(MAP_RECORD_PTR));
  if (map_ptr == NULL) myquit(OUT_OF_MEMORY,"table_maps");
  ((MAP_RECORD_PTR)map_ptr)->count = 0;
  ((MAP_RECORD_PTR)map_ptr)->size = 7;    // initial size 
  ((MAP_RECORD_PTR)map_ptr)->htable = (BPLONG_PTR)malloc(7*sizeof(BPLONG));

  picat_table_maps[i] = (BPLONG_PTR)map_ptr;
  picat_table_map_ids[i] = map_id_cp;
  return unify(map_num, MAKEINT(i));  
}

void expand_picat_table_map(MAP_RECORD_PTR mr_ptr){
    BPLONG new_htable_size,old_htable_size,i;
    BPLONG_PTR new_htable,old_htable;
	KEY_VAL_PAIR_PTR kvp_ptr, next_kvp_ptr;

    old_htable_size = mr_ptr->size;
    old_htable = mr_ptr->htable;
    new_htable_size = 3*old_htable_size;
    new_htable_size = bp_hsize(new_htable_size);

	printf("table_map expand %d\n",new_htable_size);

    new_htable = (BPLONG_PTR)malloc(sizeof(BPLONG)*new_htable_size);
    if (new_htable == NULL) return; /* stop expanding */
    for (i=0; i<new_htable_size; i++){
	  new_htable[i] = (BPLONG)NULL;
    }
    for (i=0; i<old_htable_size; i++){
	  kvp_ptr = (KEY_VAL_PAIR_PTR)old_htable[i];
	  while (kvp_ptr != NULL){
		BPLONG_PTR new_kvp_ptr_ptr;

		next_kvp_ptr = kvp_ptr->next;
		new_kvp_ptr_ptr = (BPLONG_PTR)(new_htable + bp_hashval(kvp_ptr->key)%new_htable_size);
		kvp_ptr->next = (BPLONG_PTR)FOLLOW(new_kvp_ptr_ptr);
		FOLLOW(new_kvp_ptr_ptr) = (BPLONG)kvp_ptr;
		kvp_ptr = next_kvp_ptr;
	  }
	}
    free(old_htable);
    mr_ptr->size = new_htable_size;
    mr_ptr->htable = new_htable;
}

int b_PICAT_TABLE_MAP_PUT_cc(BPLONG map_num, BPLONG key, BPLONG val){
  BPLONG i, key_cp, val_cp, this_hcode, this_ground_flag;
  BPLONG_PTR trail_top0,tmp_ptr;
  MAP_RECORD_PTR mr_ptr;
  KEY_VAL_PAIR_PTR kvp_ptr;

  DEREF_NONVAR(map_num);
  map_num = INTVAL(map_num);
  mr_ptr = (MAP_RECORD_PTR)picat_table_maps[map_num];

  initial_diff0 = (BPULONG)trail_up_addr-(BPULONG)trail_top;
  PREPARE_NUMBER_TERM(0);
  key_cp = numberVarCopyToTableArea(ta_record_ptr,key,&this_hcode,&this_ground_flag);
  if (key_cp == BP_ERROR) return BP_ERROR;
  
  kvp_ptr_ptr = mr_ptr->htable + (this_hcode % mr_ptr->size); 
  kvp_ptr = (KEY_VAL_PAIR_PTR)FOLLOW(kvp_ptr_ptr);

  while (kvp_ptr != NULL){ /* lookup */
	if (kvp_ptr->key != key_cp && !unifyNumberedTerms(kvp_ptr->key,key_cp)){
	  kvp_ptr = (KEY_VAL_PAIR_PTR)kvp_ptr->next;
	} else {
	  val_cp = numberVarCopyToTableArea(ta_record_ptr,val,&this_hcode,&this_ground_flag);
	  if (val_cp == BP_ERROR) return BP_ERROR;
	  kvp_ptr->val = val_cp;
	  goto lookup_end;
	}
  }
  // come here if lookup failed 
  ALLOCATE_FROM_NUMBERED_TERM_AREA(ta_record_ptr,tmp_ptr,sizeof(KEY_VAL_PAIR));
  if (tmp_ptr == NULL) myquit(OUT_OF_MEMORY,"table_maps");
  kvp_ptr = (KEY_VAL_PAIR_PTR)tmp_ptr;
  kvp_ptr->key = key_cp;
  kvp_ptr->val = val_cp;
  kvp_ptr->next = (BPLONG_PTR)FOLLOW(kvp_ptr_ptr);
  FOLLOW(kvp_ptr_ptr) = (BPLONG_PTR)kvp_ptr;

  mr_ptr->count++;
  if (2*mr_ptr->count > mr_ptr->size)
	expand_picat_table_map(mr_ptr);
  
lookup_end:
  trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
  UNDO_TRAILING;
  return BP_TRUE;
}

int b_PICAT_TABLE_MAP_GET_ccf(BPLONG map_num, BPLONG key, BPLONG val){
  BPLONG i, key_cp, val_cp, this_hcode, this_ground_flag;
  BPLONG_PTR trail_top0;
  MAP_RECORD_PTR mr_ptr;
  KEY_VAL_PAIR_PTR kvp_ptr;

  DEREF_NONVAR(map_num);
  map_num = INTVAL(map_num);
  mr_ptr = (MAP_RECORD_PTR)picat_table_maps[map_num];

  initial_diff0 = (BPULONG)trail_up_addr-(BPULONG)trail_top;
  PREPARE_NUMBER_TERM(0);
  key_cp = numberVarCopyToTableArea(ta_record_ptr,key,&this_hcode,&this_ground_flag);
  if (key_cp == BP_ERROR) return BP_ERROR;

  kvp_ptr_ptr = mr_ptr->htable + (this_hcode % mr_ptr->size); 
  kvp_ptr = (KEY_VAL_PAIR_PTR)FOLLOW(kvp_ptr_ptr);

  while (kvp_ptr != NULL){ /* lookup */
	if (kvp_ptr->key != key_cp && !unifyNumberedTerms(kvp_ptr->key,key_cp)){
	  kvp_ptr = (KEY_VAL_PAIR_PTR)kvp_ptr->next;
	} else {
	  PREPARE_UNNUMBER_TERM(local_top);
	  trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
	  UNDO_TRAILING;
	  return unify(val,unnumberVarTabledTerm(kvp_ptr->val));
	}
  }
  trail_top0 = (BPLONG_PTR)((BPULONG)trail_up_addr-initial_diff0);
  UNDO_TRAILING;
  return BP_FALSE;
}
