#ifndef SYMBOL_H
#define SYMBOL_H

typedef SecondTokenType SymbolSecondType;

typedef enum Symbol_type
{
	SYMBOL_CLASS, 
	SYMBOL_FUNCTION_CONSTRUCTOR, SYMBOL_FUNCTION_FUNCTION, SYMBOL_FUNCTION_METHOD, 
	SYMBOL_VAR_STATIC, SYMBOL_VAR_FIELD, SYMBOL_VAR_LOCAL, SYMBOL_VAR_PARAMETER
}symbol_type;

extern struct hash_struct_info sym_global_hash;

int symbol_table_create();
struct symbol_info * symbol_search(struct symbol_table_entry *token);
int symbol_insert(symbol_type type, SymbolSecondType secondtype, struct symbol_table_entry *token);
int symbol_table_init();
int symbol_table_free();

#endif