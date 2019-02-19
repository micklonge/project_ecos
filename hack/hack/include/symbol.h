/*
 ***************************************************************
 *
 *         FILE NAME      : symbol.h
 *         ORIGINATOR     : ÕıÓ»—Ô(micklongen)
 *         DATE OF ORIGIN : 
 *
 ***************************************************************
 */

#ifndef SYMBOL_H
#define SYMBOL_H

#define MAX_SYMBOL_NAME 100
#define SYMBOL_SIZE 23

/* the symbol info */
struct symbol_info
{
	char *symbol_name;
	int addr;
};

/* the sturcture in the symbol table */
struct symbol_table_entry
{
	struct DListNode link;
	char *symbol_name;
	int addr;
};

extern struct hash_struct_info symbol_hash;

int symbol_init();
int hack_hash(void *);
int hack_cmp(void *, void *);
void get_variable(char *str);
struct symbol_table_entry* sym_search(char *str);
int sym_insert(struct symbol_info*);
int symbol_table_free();
int get_symbol_addr(char *str);
int sym_print();

#endif