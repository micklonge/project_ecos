#include "stdafx.h"

#include "..\..\common\inc\common.h"
#include "..\..\common\inc\log.h"
#include "..\..\common\inc\list.h"
#include "..\..\common\inc\hash.h"
#include "..\..\common\inc\lib.h"
#include "include\jack.h"
#include "include\lex.h"
#include "include\symbol.h"

#pragma warning (disable:4996)                                   // ÆÁ±Î¾¯¸æ

struct symbol_info
{
	struct DListNode link;
	symbol_type type;
	SymbolSecondType secondtype;
	char sym_name[MAX_SYMBOL_NAME_LEN + 1];
};

struct symbol_table_struct
{
	struct DListNode link;
	struct hash_struct_info sym_table;
};
struct DListHeader symbol_header;


static int jack_symbol_hash(void *node)
{
	char *str = ((struct symbol_info *)node)->sym_name;
	int i, sum = 0, len = (int)strlen(str);
    
    for(i = 0;i < len;i ++)
          sum += str[i];

    return sum;
}

static int jack_symbol_cmp(void *src, void *des)
{
	struct symbol_info *p1 = (struct symbol_info*)src;
	struct symbol_info *p2 = (struct symbol_info*)des;

	return strcmp((char*)p1->sym_name, (char*)p2->sym_name);
}

static int jack_symbol_free(void *addr)
{
	struct symbol_info *p = (struct symbol_info*)addr;

	com_free(p);

	return 0;
}

int symbol_table_create()
{
	struct symbol_table_struct *symbol_table = NULL;

	symbol_table = (struct symbol_table_struct *)com_malloc(sizeof(struct symbol_table_struct));
	hash_init(&symbol_table->sym_table, jack_symbol_hash, jack_symbol_cmp, jack_symbol_free);
	DList_add_head(&symbol_header, &symbol_table->link);

	return 0;
}

struct symbol_info * symbol_search(struct symbol_table_entry *token)
{
	struct symbol_table_struct *sym_table = NULL;
	struct symbol_info entry;
	struct symbol_info *p;

	sym_table = (struct symbol_table_struct *)DList_get_head(&symbol_header);
	while (sym_table != NULL)
	{
		strcpy(entry.sym_name, token->sym_name);
		p = (struct symbol_info *)Hash_list_search(&sym_table->sym_table, &entry.link);
		if (p != 0)
			return p;
		sym_table = (struct symbol_table_struct *)DList_get_next(&symbol_header, &sym_table->link);
	}

	return NULL;
}

int symbol_insert(symbol_type type, SymbolSecondType secondtype, struct symbol_table_entry *token)
{
	struct symbol_info *entry = NULL;
	struct symbol_table_struct *sym_table = NULL;

	sym_table = (struct symbol_table_struct *)DList_get_head(&symbol_header);
	if (sym_table != NULL)
	{
		entry = (struct symbol_info *)com_malloc(sizeof(struct symbol_info));
		entry->type = type;
		entry->secondtype = secondtype;
		strcpy(entry->sym_name, token->sym_name);

		Hash_list_insert(&sym_table->sym_table, &entry->link);
	}
	else
	{
		log_print("The symbol insert failed.\n");
		exit(1);
	}

	return 0;
}

int symbol_table_init()
{
	initialize_Dlist_header(&symbol_header);
	symbol_table_create();

	return 0;
}

int symbol_table_free()
{
	struct symbol_table_struct *sym_table;

	sym_table = (struct symbol_table_struct *)DList_get_head(&symbol_header);
	if (sym_table != NULL)
	{
		DList_delete_node(&symbol_header, &sym_table->link);
		Hash_free(&sym_table->sym_table);
		com_free(sym_table);
	}

	return 0;
}