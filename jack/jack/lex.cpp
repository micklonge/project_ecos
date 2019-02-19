#include "stdafx.h"

#include "..\..\common\inc\common.h"
#include "..\..\common\inc\log.h"
#include "..\..\common\inc\list.h"
#include "..\..\common\inc\hash.h"
#include "..\..\common\inc\lib.h"
#include "include\jack.h"
#include "include\lex.h"
#include "include\symbol.h"

#define is_variable_start(ch) (isalpha(ch) || ch == '_')
#define is_variable(ch) (is_variable_start(ch) || isdigit(ch))

struct hash_struct_info sym_global_hash;

struct symbol_table_entry key_words[] = 
{
	{{NULL, NULL}, "class", T_KEYWORD, K_CLASS},
	{{NULL, NULL}, "method", T_KEYWORD, K_METHOD},
	{{NULL, NULL}, "function", T_KEYWORD, K_FUNCTION},
	{{NULL, NULL}, "constructor", T_KEYWORD, K_CONSTRUCTOR},

	{{NULL, NULL}, "int", T_KEYWORD, K_INT},
	{{NULL, NULL}, "boolean", T_KEYWORD, K_BOOLEAN},
	{{NULL, NULL}, "char", T_KEYWORD, K_CHAR},
	{{NULL, NULL}, "void", T_KEYWORD, K_VOID},

	{{NULL, NULL}, "var", T_KEYWORD, K_VAR},
	{{NULL, NULL}, "static", T_KEYWORD, K_STATIC},
	{{NULL, NULL}, "field", T_KEYWORD, K_FIELD},

	{{NULL, NULL}, "let", T_KEYWORD, K_LET},
	{{NULL, NULL}, "do", T_KEYWORD, K_DO},
	{{NULL, NULL}, "if", T_KEYWORD, K_IF},
	{{NULL, NULL}, "else", T_KEYWORD, K_ELSE},
	{{NULL, NULL}, "while", T_KEYWORD, K_WHILE},
	{{NULL, NULL}, "return", T_KEYWORD, K_RETURN},

	{{NULL, NULL}, "true", T_KEYWORD, K_TRUE},
	{{NULL, NULL}, "false", T_KEYWORD, K_FALSE},
	{{NULL, NULL}, "null", T_KEYWORD, K_NULL},

	{{NULL, NULL}, "this", T_KEYWORD, K_THIS}
};
#define NUM_OF_KEY_WORD      (sizeof(key_words) / sizeof(struct symbol_table_entry))

/*--------------------------------------------------------------------------------*
 *                         read a line                                            *
 *--------------------------------------------------------------------------------*/
#define MAX_BUFFER_INC 10000

char * read_line()
{
	static int flag = 0;

	int buffer_size = 0;
	char *buffer = NULL;
	char *p = NULL, *q = NULL, *t = NULL;

	buffer_size = MAX_BUFFER_INC;
	while (1)
	{
		buffer = (char *)com_malloc(sizeof(char) * buffer_size); 
		fgets(buffer, buffer_size, Input_file_id);
		
		p = buffer;
		if (flag == 1)                   /* ignore block comments */
		{
			while (LINE_NOT_END(*p))
			{
				if (p[0] == '*' && p[1] == '/')
				{
					p += 2;
					flag = 0;
					break;
				}
				++p;
			}
			
		}

		while (*p == ' ')                /* ignore the space before the line */
			++p;

		q = p;
		while (LINE_NOT_END(*q))         
		{
			if (q[0] == '/' && q[1] == '/')    /* ignore the line comments */
			{
				*q = '\0';
				break;
			}

			if (q[0] == '/' && q[1] == '*')    /* ignore block comments */
			{
				t = q;
				flag = 1;
				q += 2;
				continue;
			}

			if (q[0] == '*' && q[1] == '/')
			{
				q += 2;
				flag = 0;
				do
				{
					*t++ = *q++;
				}while(LINE_NOT_END(*q));
				q = p;
				continue;
			}

			++q;
		}

		if (flag)
			*t = '\0';

		if (LINE_IS_END(*p))      /* ignore blank line */
		{
			com_free(buffer);
			continue;
		}
		else
			break;
	}

	log_print("The contents of read_line: %s\n", buffer);

	return buffer;
}

static int jack_lex_hash(void *node)
{
	char *str = ((struct symbol_table_entry *)node)->sym_name;
	int i, sum = 0, len = (int)strlen(str);
    
    for(i = 0;i < len;i ++)
          sum += str[i];

    return sum;
}

static int jack_lex_cmp(void *src, void *des)
{
	struct symbol_table_entry *p1 = (struct symbol_table_entry*)src;
	struct symbol_table_entry *p2 = (struct symbol_table_entry*)des;

	return strcmp((char*)p1->sym_name, (char*)p2->sym_name);
}

static int lex_init()
{
	int i;

	hash_init(&sym_global_hash, jack_lex_hash, jack_lex_cmp, NULL);
	for (i = 0; i < NUM_OF_KEY_WORD; ++i)
		Hash_list_insert(&sym_global_hash, &key_words[i].link);

	return 0;
}

static struct DListHeader * get_token_list(struct DListHeader *header, char *line)
{
	char *p = line, *q = NULL;
	struct symbol_table_entry *entry = NULL, *token = NULL;

	while (LINE_NOT_END(*p))
	{
		if ((*p == ' ') || (*p == 9))
		{
			++p;
			continue;
		}

		entry = (struct symbol_table_entry *)com_malloc(sizeof(struct symbol_table_entry));
		q = entry->sym_name;

		if (is_variable_start(*p))
		{
			do
			{
				*q++ = *p++;
			}while(is_variable(*p));
			*q = '\0';
			
			if ((token = (struct symbol_table_entry *)Hash_list_search(&sym_global_hash, &entry->link)) == NULL)
				entry->token_type = T_IDENTIFIER;
			else
			{
				entry->token_type = token->token_type;
				entry->second_token_type = token->second_token_type;
			}
		}
		else if (isdigit(*p))
		{
			do
			{
				*q++ = *p++;
			}while(isdigit(*p));
			*q = '\0';
			entry->token_type = T_INT_CONST;
		}
		else if(*p == '\"')
		{
			++p;
			do
			{
				*q++ = *p++;
			}while(*p != '\"');
			*q = '\0';
			entry->token_type = T_STRING_CONST;
			++p;
		}
		else 
		{
			entry->token_type = T_SYMBOL;
			switch(*p)
			{
			case '(':
				entry->second_token_type = S_LPARENT;
				break;
			case ')':
				entry->second_token_type = S_RPARENT;
				break;
			case '[':
				entry->second_token_type = S_LBRACKET;
				break;
			case ']':
				entry->second_token_type = S_RBRACKET;
				break;
			case '{':
				entry->second_token_type = S_LBRACE;
				break;
			case '}':
				entry->second_token_type = S_RBRACE;
				break;
			case ',':
				entry->second_token_type = S_COMMA;
				break;
			case ';':
				entry->second_token_type = S_SEMICOLON;
				break;
			case '=':
				entry->second_token_type = S_EQUAL_SIGN;
				break;
			case '.':
				entry->second_token_type = S_DOT;
				break;
			case '+':
				entry->second_token_type = S_ADD;
				break;
			case '-':
				entry->second_token_type = S_SUB;
				break;
			case '*':
				entry->second_token_type = S_MUL;
				break;
			case '/':
				entry->second_token_type = S_DIV;
				break;
			case '&':
				entry->second_token_type = S_AND;
				break;
			case '|':
				entry->second_token_type = S_OR;
				break;
			case '~':
				entry->second_token_type = S_NOT;
				break;
			case '<':
				entry->second_token_type = S_BIG;
				break;
			case '>':
				entry->second_token_type = S_SMALL;
				break;
			}
			*q++ = *p++;
			*q = '\0';
		}

		DList_add_tail(header, &entry->link);
	}

	return header;
}

struct symbol_table_entry * get_token()
{
	static int get_token_first = 1;
	static struct DListHeader list_header;
	static struct DListNode *node= NULL;
	struct symbol_table_entry *entry = NULL;
	char *line = NULL;

	if (get_token_first == 1)
	{
		get_token_first = 0;
		initialize_Dlist_header(&list_header);
		lex_init();
	}

	while (node == NULL || ((node = DList_get_next(&list_header, node)) == NULL))
	{
		while ((node = DList_get_head(&list_header)) != NULL)
		{
			node = DList_delete_node(&list_header, node);
			entry = (struct symbol_table_entry *)node;
			com_free(entry);
		}

		line = read_line();
		get_token_list(&list_header, line);
		com_free(line);
		if ((node = DList_get_head(&list_header)) != NULL)
			break;
	}

	entry = (struct symbol_table_entry *)node;

	return entry;
}