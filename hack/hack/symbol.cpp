/*
 ***************************************************************
 *
 *         FILE NAME      : symbol.cpp
 *         ORIGINATOR     : ÕıÓ»—Ô(micklongen)
 *         DATE OF ORIGIN : 
 *
 ***************************************************************
 */
#include "stdafx.h"
#include <stdio.h>

#include "..\..\common\inc\log.h"
#include "..\..\common\inc\list.h"
#include "..\..\common\inc\hash.h"
#include "..\..\common\inc\lib.h"
#include "include\front.h"
#include "include\symbol.h"

/*
 *******************************************************************
 * global variable
 *******************************************************************
 */
struct hash_struct_info symbol_hash;

/*
 *******************************************************************
 * the pre define symbol
 *******************************************************************
 */
#define MAX_PRE_SYMBOL 23
struct symbol_info pre_symbol[] = {{"SP",0},{"LCL",1},{"ARG",2},{"THIS",3},
                                   {"THAT",4},{"R0",0},{"R1",1},{"R2",2},
                                   {"R3",3},{"R4",4},{"R5",5},{"R6",6},
                                   {"R7",7},{"R8",8},{"R9",9},{"R10",10},
                                   {"R11",11},{"R12",12},{"R13",13},{"R14",14},
                                   {"R15",15},{"SCREEN",16384},{"KBD",24576}};

int symbol_init()
{
	int i;

	hash_init(&symbol_hash, hack_hash, hack_cmp, NULL);

	for (i = 0; i < MAX_PRE_SYMBOL; ++i)
	{
		log_print("The symbol name:%s.\n", pre_symbol[i].symbol_name);
		sym_insert(&pre_symbol[i]);
	}

	return 0;
}

int hack_hash(void *node)
{
	char *str = ((struct symbol_table_entry *)node)->symbol_name;
	int i,sum = 0,len = (int)strlen(str);
    
    for(i = 0;i < len;i ++)
          sum += str[i];
    sum = sum % SYMBOL_SIZE;

    return sum;
}

int hack_cmp(void *src, void *des)
{
	struct symbol_table_entry *p1 = (struct symbol_table_entry*)src;
	struct symbol_table_entry *p2 = (struct symbol_table_entry*)des;

	return strcmp((char*)p1->symbol_name, (char*)p2->symbol_name);
}

/* search symbol */
struct symbol_table_entry* sym_search(char *str)
{
	struct symbol_table_entry entry;

	entry.symbol_name = str;

	return (struct symbol_table_entry*)Hash_list_search(&symbol_hash, &(entry.link));
}

int sym_insert(struct symbol_info *sym_info)
{
	struct symbol_table_entry *entry = NULL;
	struct symbol_table_entry *plink = NULL;

	plink = sym_search(sym_info->symbol_name);
	if (plink == NULL)
	{
		entry = (struct symbol_table_entry *)com_malloc(sizeof(struct symbol_table_entry));
		entry->symbol_name = (char *)com_malloc(strlen(sym_info->symbol_name) + 1);

		strcpy(entry->symbol_name, sym_info->symbol_name);
		entry->addr = sym_info->addr;
		Hash_list_insert(&symbol_hash, &(entry->link));
	}

	return 0;
}

int symbol_table_free()
{
	Hash_free(&symbol_hash);
	return 0;
}

int get_symbol_addr(char *str)
{
	struct symbol_table_entry *entry = NULL;

	entry = sym_search(str);
	if (entry != NULL)
		return entry->addr;
	else 
	{
		entry = (struct symbol_table_entry *)com_malloc(sizeof(struct symbol_table_entry));
		entry->symbol_name = (char *)com_malloc(strlen(str) + 1);

		strcpy(entry->symbol_name, str);
		entry->addr = data_addr++;
		Hash_list_insert(&symbol_hash, &(entry->link));

		return entry->addr;
	}
}

int sym_print()
{
	int i;
	struct DListNode *plink;
	struct DListHeader *header;

	for (i = 0; i < SYMBOL_SIZE; ++i)
	{
		log_print("The NO. %d:\n", i);
		log_print("The number of node in the list: %d\n", symbol_hash.hash_list[i].count);
	    header = &symbol_hash.hash_list[i];
	    plink = DList_get_next(header, &(header->head));
	    while(plink != NULL)
	    {
		    log_print("    %s\n", ((struct symbol_table_entry *)plink)->symbol_name);
		    plink = DList_get_next(header, plink);
	    }		
	}

	return 0;
}