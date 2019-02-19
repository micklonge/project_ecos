#include "stdafx.h"

#include "..\..\common\inc\common.h"
#include "..\..\common\inc\log.h"
#include "..\..\common\inc\list.h"
#include "..\..\common\inc\hash.h"
#include "..\..\common\inc\lib.h"
#include "include\jack.h"
#include "include\jack_fsm.h"
#include "include\lex.h"
#include "include\symbol.h"
#include "include\syntax.h"

/*********************************************************************************
 *                           function prototype                                  *
 *********************************************************************************/
static struct symbol_table_entry * jack_expressionList(void *data);
static struct symbol_table_entry * jack_subroutineCall(void *data);
static struct symbol_table_entry * jack_term(void *data);
static struct symbol_table_entry * jack_expression(void *data);

static struct symbol_table_entry * jack_let_statement(void *data);
static struct symbol_table_entry * jack_do_statement(void *data);
static struct symbol_table_entry * jack_if_statement(void *data);
static struct symbol_table_entry * jack_while_statement(void *data);
static struct symbol_table_entry * jack_return_statement(void *data);
static struct symbol_table_entry * jack_statement(void *data);

static int jack_class_var_dec(void *data);
static int jack_class_subroutine_body(void *data);
static int jack_class_subroutine_dec(void *data);
static int jack_class(void *data);

/*********************************************************************************
 *                              expresion                                        *
 *********************************************************************************/
#define     IS_OPERATION_SYMBOL(x)             (((x) > S_OP_BEGIN) && ((x) < S_OP_END))
#define     IS_UNARYOPERATION_SYMBOL(x)        (((x) == S_NOT) || ((x) == S_NEG))

#define     IS_KEYWORDS_CONSTANT(x)            (((x) > K_KEYWORDS_CONSTANT_BEGIN) && ((x) < K_KEYWORDS_CONSTANT_END))

/*********************************************************************************
 *                              statements                                       *
 *********************************************************************************/
#define     IS_STATEMENT(x) (((x) > K_STATEMENT_BEGIN) && ((x) < K_STATEMENT_END))

/*********************************************************************************
 *                           procedure structure                                 *
 *********************************************************************************/
#define 	SYN_CLASS                      0x00000001
#define     SYN_CLASS_VAR_DEC              0x00000002
#define     SYN_CLASS_SUBROOTINE_DEC       0x00000003
#define 	SYN_END                        0x00000004

#define     EMBEDDED_TYPE_CHECK(x) ((x) == K_INT || (x) == K_BOOLEAN || (x) == K_CHAR)
#define     TYPE_CHECK(x) (((x)->token_type == T_IDENTIFIER) || (((x)->token_type == T_KEYWORD) && EMBEDDED_TYPE_CHECK(x->second_token_type)))

struct JACK_FSM syn_fsm[] = 
{
	{NULL, NULL},
	{jack_class, SYN_CLASS_VAR_DEC},
	{jack_class_var_dec, SYN_CLASS_VAR_DEC},
	{jack_class_subroutine_dec, SYN_CLASS_SUBROOTINE_DEC}
};

/*********************************************************************************
 *                                expreesion                                     *
 *********************************************************************************/
static struct symbol_table_entry * jack_expressionList(void *data)
{
	struct symbol_table_entry * entry = (symbol_table_entry *)data;

	fprintf(Output_file_id, "<expressionList>\n");

	while(1)
	{
		entry = jack_expression(entry);
		if ((entry->token_type != T_SYMBOL) || entry->second_token_type != S_COMMA)
			break;
		fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);
		entry = get_token();
	}

	fprintf(Output_file_id, "</expressionList>\n");

	return entry;
}

static struct symbol_table_entry * jack_subroutineCall(void *data)
{
	struct symbol_table_entry * entry = (symbol_table_entry *)data;

	if (entry->token_type != T_IDENTIFIER)
	{
		log_print("identifier expected.\n");
		exit(1);
	}
	fprintf(Output_file_id, "<identifier> %s </identifier>\n", entry->sym_name);

	entry = get_token();
	if ((entry->token_type == T_SYMBOL) && (entry->second_token_type == S_DOT))
	{
		fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);
		entry = get_token();
		if (entry->token_type != T_IDENTIFIER)
		{
			log_print("identifier expected.\n");
			exit(1);
		}
		fprintf(Output_file_id, "<identifier> %s </identifier>\n", entry->sym_name);
		entry = get_token();
	}

	if (!((entry->token_type == T_SYMBOL) && (entry->second_token_type == S_LPARENT)))
	{
		log_print("\'(\' expected.\n");
		exit(1);
	}
	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);

	entry = get_token();
	entry = jack_expressionList(entry);

	if (!((entry->token_type == T_SYMBOL) && (entry->second_token_type == S_RPARENT)))
	{
		log_print("\')\' expected.\n");
		exit(1);
	}
	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);

	return get_token();
}

static struct symbol_table_entry * jack_term(void *data)
{
	struct symbol_table_entry * entry = (symbol_table_entry *)data;
	struct symbol_table_struct *sym_table_entry = NULL;

	fprintf(Output_file_id, "<term>\n");

	if (entry->token_type == T_INT_CONST)
	{
		fprintf(Output_file_id, "<integerConstant> %s </integerConstant>\n", entry->sym_name);
		entry = get_token();
	}
	else if (entry->token_type == T_STRING_CONST)
	{
		fprintf(Output_file_id, "<stringConstant> %s </stringConstant>\n", entry->sym_name);
		entry = get_token();
	}
	else if ((entry->token_type == T_KEYWORD) && (IS_KEYWORDS_CONSTANT(entry->second_token_type)))
	{
		fprintf(Output_file_id, "<keyword> %s </keyword>\n", entry->sym_name);
		entry = get_token();
	}
	else if ((entry->token_type == T_SYMBOL) && (entry->second_token_type == S_LPARENT))
	{
		fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);
		entry = jack_expression(get_token());
		fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);
		entry = get_token();
	}
	else if ((entry->token_type == T_SYMBOL) && (IS_UNARYOPERATION_SYMBOL(entry->second_token_type)))
	{
		fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);
		entry = jack_term(get_token());
	}
	else if (entry->token_type == T_IDENTIFIER)
	{
		sym_table_entry = (symbol_table_struct *)symbol_search(entry);
		if (sym_table_entry == NULL)
			entry = jack_subroutineCall(entry);
		else
		{
			fprintf(Output_file_id, "<identifier> %s </identifier>\n", entry->sym_name);
			entry = get_token();
			if ((entry->token_type == T_SYMBOL) && (entry->second_token_type == S_LBRACKET))
			{
				fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);
				entry = jack_expression(get_token());
				fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);
				entry = get_token();
			}
		}
	}
	
	fprintf(Output_file_id, "</term>\n");

	return entry;
}

static struct symbol_table_entry * jack_expression(void *data)
{
	struct symbol_table_entry * entry = (symbol_table_entry *)data;

	fprintf(Output_file_id, "<expression>\n");
	while(1)
	{
		entry = jack_term(entry);
		if ((entry->token_type != T_SYMBOL) || !IS_OPERATION_SYMBOL(entry->second_token_type))
			break;
		fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);
		entry = get_token();
	}
	fprintf(Output_file_id, "</expression>\n");

	return entry;
}


/*********************************************************************************
 *                                statement                                      *
 *********************************************************************************/
static struct symbol_table_entry * jack_let_statement(void *data)
{
	struct symbol_table_entry * entry = (symbol_table_entry *)data;

	fprintf(Output_file_id, "<letStatement>\n");
	fprintf(Output_file_id, "<keyword> let </keyword>\n");

	entry = get_token();
	if (entry->token_type != T_IDENTIFIER)
	{
		log_print("VarName is expected.\n");
		exit(1);
	}
	fprintf(Output_file_id, "<identifier> %s </identifier>\n", entry->sym_name);

	entry = get_token();
	if ((entry->token_type == T_SYMBOL) && (entry->second_token_type == S_LBRACKET))
	{
		fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);
		entry = jack_expression(get_token());
		fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);
		entry = get_token();
	}
	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);

	entry = jack_expression(get_token());
	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);

	fprintf(Output_file_id, "</letStatement>\n");

	return  get_token();
}

static struct symbol_table_entry * jack_do_statement(void *data)
{
	struct symbol_table_entry * entry = (symbol_table_entry *)data;

	fprintf(Output_file_id, "<doStatement>\n");
	fprintf(Output_file_id, "<keyword> do </keyword>\n");

	entry = get_token();
	entry = jack_subroutineCall(entry);

	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);

	fprintf(Output_file_id, "</doStatement>\n");

	return get_token();
}

static struct symbol_table_entry * jack_if_statement(void *data)
{
	struct symbol_table_entry * entry = (symbol_table_entry *)data;

	fprintf(Output_file_id, "<ifStatement>\n");
	fprintf(Output_file_id, "<keyword> if </keyword>\n");

	entry = get_token();
	if (!((entry->token_type == T_SYMBOL) && (entry->second_token_type == S_LPARENT)))
	{
		log_print("\'(\' expected.\n");
		exit(1);
	}
	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);

	entry = jack_expression(get_token());

	if (!((entry->token_type == T_SYMBOL) && (entry->second_token_type == S_RPARENT)))
	{
		log_print("\')\' expected.\n");
		exit(1);
	}
	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);

	entry = get_token();
	if (!((entry->token_type == T_SYMBOL) && (entry->second_token_type == S_LBRACE)))
	{
		log_print("\'{\' expected.\n");
		exit(1);
	}
	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);

	entry = jack_statement(get_token());

	if (!((entry->token_type == T_SYMBOL) && (entry->second_token_type == S_RBRACE)))
	{
		log_print("\'}\' expected.\n");
		exit(1);
	}
	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);
	entry = get_token();

	if (entry->second_token_type == K_ELSE)
		while ((entry = get_token()) && !(entry->token_type == T_SYMBOL && entry->second_token_type == S_RBRACE));

	fprintf(Output_file_id, "</ifStatement>\n");

	return entry;
}

static struct symbol_table_entry * jack_while_statement(void *data)
{
	struct symbol_table_entry * entry = (symbol_table_entry *)data;

	fprintf(Output_file_id, "<whileStatement>\n");
	fprintf(Output_file_id, "<keyword> while </keyword>\n");

	entry = get_token();
	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);

	entry = jack_expression(get_token());
	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);

	entry = get_token();
	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);

	entry = jack_statement(get_token());
	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);

	fprintf(Output_file_id, "</whileStatement>\n");

	return get_token();
}

static struct symbol_table_entry * jack_return_statement(void *data)
{
	struct symbol_table_entry * entry = (symbol_table_entry *)data;

	fprintf(Output_file_id, "<returnStatement>\n");
	fprintf(Output_file_id, "<keyword> return </keyword>\n");

	entry = get_token();
	if (!(entry->token_type == T_SYMBOL && entry->second_token_type == S_SEMICOLON))
		entry = jack_expression(entry);

	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);

	fprintf(Output_file_id, "</returnStatement>\n");

	return get_token();
}

static struct symbol_table_entry * jack_statement(void *data)
{
	struct symbol_table_entry *entry = (struct symbol_table_entry *)data;
	
	fprintf(Output_file_id, "<statements>\n");
	while (IS_STATEMENT(entry->second_token_type))
	{
		switch(entry->second_token_type)
		{
		case K_LET:
			entry = jack_let_statement(entry);
			break;
		case K_DO:
			entry = jack_do_statement(entry);
			break;
		case K_IF:
			entry = jack_if_statement(entry);
			break;
		case K_WHILE:
			entry = jack_while_statement(entry);
			break;
		case K_RETURN:
			entry = jack_return_statement(entry);
			break;
		default:
			break;
		}
	}
	fprintf(Output_file_id, "</statements>\n");
	return entry;
}


/*********************************************************************************
 *                           function implementtion                              *
 *********************************************************************************/
static int jack_class_var_dec(void *data)
{
	symbol_type type;
	SymbolSecondType secondtype;
	struct symbol_table_entry *entry = (struct symbol_table_entry *)data;

	if (entry->token_type != T_KEYWORD || ((entry->second_token_type != K_STATIC) && (entry->second_token_type != K_FIELD)))
		return 1;

	fprintf(Output_file_id, "<classVarDec>\n");

	fprintf(Output_file_id, "<keyword> %s </keyword>\n", entry->sym_name);
	if (entry->second_token_type == K_STATIC)
		type = SYMBOL_VAR_STATIC;
	else
		type = SYMBOL_VAR_FIELD;

	entry = get_token();
	if (!TYPE_CHECK(entry))
	{
		log_print("type expected\n");
		exit(1);
	}

	if (entry->token_type == T_IDENTIFIER)
	{
		fprintf(Output_file_id, "<identifier> %s </identifier>\n", entry->sym_name);
		secondtype = K_CLASS;
	}
	else
	{
		fprintf(Output_file_id, "<keyword> %s </keyword>\n", entry->sym_name);
		secondtype = entry->second_token_type;
	}

	entry = get_token();
	while((entry->token_type != T_SYMBOL) || (entry->second_token_type != S_SEMICOLON))
	{
		if (entry->token_type == T_SYMBOL)
			fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);
		else
		{
			fprintf(Output_file_id, "<identifier> %s </identifier>\n", entry->sym_name);
			symbol_insert(type, secondtype, entry);
		}
		entry = get_token();
	}

	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);
	fprintf(Output_file_id, "</classVarDec>\n");

	return 0;
}

static int jack_class_subroutine_body(void *data)
{
	symbol_type type;
	SymbolSecondType secondtype;
	struct symbol_table_entry *entry = (struct symbol_table_entry *)data;

	fprintf(Output_file_id, "<subroutineBody>\n");
	type = SYMBOL_VAR_LOCAL;

	entry = get_token();
	if (entry->token_type != T_SYMBOL || entry->second_token_type != S_LBRACE)
	{
		log_print("\'{\' expected.\n");
		exit(1);
	}

	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);

	entry = get_token();
	while (entry->second_token_type == K_VAR)
	{
		fprintf(Output_file_id, "<varDec>\n");
		fprintf(Output_file_id, "<keyword> %s </keyword>\n", entry->sym_name);

		entry = get_token();
		if (!TYPE_CHECK(entry))
		{
			log_print("type expected.\n");
			exit(1);
		}

		if (entry->token_type == T_IDENTIFIER)
		{
			fprintf(Output_file_id, "<identifier> %s </identifier>\n", entry->sym_name);
			secondtype = K_CLASS;
		}
		else
		{
			fprintf(Output_file_id, "<keyword> %s </keyword>\n", entry->sym_name);
			secondtype = entry->second_token_type;
		}

		while (entry->second_token_type != S_SEMICOLON)
		{
			entry = get_token();
			if (entry->token_type != T_IDENTIFIER)
			{
				log_print("identifier expected.\n");
				exit(1);
			}
			fprintf(Output_file_id, "<identifier> %s </identifier>\n", entry->sym_name);
			symbol_insert(type, secondtype, entry);

			entry = get_token();
			fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);
		}

		fprintf(Output_file_id, "</varDec>\n");

		entry = get_token();
	}

	entry = jack_statement(entry);

	if (entry->token_type != T_SYMBOL || entry->second_token_type != S_RBRACE)
	{
		log_print("\'}\' expected.\n");
		exit(1);
	}

	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);

	fprintf(Output_file_id, "</subroutineBody>\n");
	return 0;
}

static int jack_class_subroutine_dec(void *data)
{
	symbol_type type;
	SymbolSecondType secondtype;
	struct symbol_table_entry *entry = (struct symbol_table_entry *)data;

	if (entry->token_type != T_KEYWORD || ((entry->second_token_type != K_METHOD) && (entry->second_token_type != K_FUNCTION) && (entry->second_token_type != K_CONSTRUCTOR)))
		return 1;

	symbol_table_create();

	fprintf(Output_file_id, "<subroutineDec>\n");
	fprintf(Output_file_id, "<keyword> %s </keyword>\n", entry->sym_name);

	if (entry->second_token_type == K_METHOD)
		type = SYMBOL_FUNCTION_METHOD;
	else if (entry->second_token_type == K_FUNCTION)
		type = SYMBOL_FUNCTION_FUNCTION;
	else
		type = SYMBOL_FUNCTION_CONSTRUCTOR;

	entry = get_token();
	if (!TYPE_CHECK(entry) && (entry->second_token_type != K_VOID))
	{
		log_print("type expected.\n");
		exit(1);
	}

	if (entry->token_type == T_IDENTIFIER)
	{
		fprintf(Output_file_id, "<identifier> %s </identifier>\n", entry->sym_name);
		secondtype = K_CLASS;
	}
	else
	{
		fprintf(Output_file_id, "<keyword> %s </keyword>\n", entry->sym_name);
		secondtype = entry->second_token_type;
	}

	entry = get_token();
	if (entry->token_type != T_IDENTIFIER)
	{
		log_print("function name expected.\n");
		exit(1);
	}

	fprintf(Output_file_id, "<identifier> %s </identifier>\n", entry->sym_name);
	symbol_insert(type, secondtype, entry);

	entry = get_token();
	if (entry->token_type != T_SYMBOL || entry->second_token_type != S_LPARENT)
	{
		log_print("\'(\' expected");
		exit(1);
	}

	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);

	fprintf(Output_file_id, "<parameterList>\n");

	entry = get_token();
	type = SYMBOL_VAR_PARAMETER;
	while(entry->second_token_type != S_RPARENT)
	{
		if (!TYPE_CHECK(entry))
		{
			log_print("type expected.\n");
			exit(1);
		}
		fprintf(Output_file_id, "<keyword> %s </keyword>\n", entry->sym_name);
		secondtype = entry->second_token_type;

		entry = get_token();
		if (entry->token_type != T_IDENTIFIER)
		{
			log_print("identifier expected.\n");
			exit(1);
		}
		fprintf(Output_file_id, "<identifier> %s </identifier>\n", entry->sym_name);
		symbol_insert(type, secondtype, entry);

		entry = get_token();
		if (entry->second_token_type != S_RPARENT)
			fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);
		else
			break;

		entry = get_token();
	}

	fprintf(Output_file_id, "</parameterList>\n");
	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);

	jack_class_subroutine_body(NULL);

	fprintf(Output_file_id, "</subroutineDec>\n");

	symbol_table_free();

	return 0;
}

static int jack_class(void *data)
{
	struct symbol_table_entry *entry;
	int current_state = SYN_CLASS_VAR_DEC;
	int error_no;

	entry = get_token();
	if (entry->token_type != T_KEYWORD || entry->second_token_type != K_CLASS)
	{
		log_print("\"class\" expected.\n");
		exit(1);
	}

	fprintf(Output_file_id, "<class>\n");
    fprintf(Output_file_id, "<keyword> %s </keyword>\n",entry->sym_name);

	entry = get_token();
	if (entry->token_type != T_IDENTIFIER)
	{
		log_print("Token name expected.\n");
		exit(1);
	}

	fprintf(Output_file_id, "<identifier> %s </identifier>\n", entry->sym_name);
	symbol_insert(SYMBOL_CLASS, A_NULL, entry);

	if (strncmp(entry->sym_name, output_file_name, (strlen(output_file_name) - strlen(suffix_output_file_name))) != 0)
	{
		log_print("The classname is not corresponding to filename.\n");
		exit(1);
	}

	entry = get_token();
	if (entry->token_type != T_SYMBOL || entry->second_token_type != S_LBRACE)
	{
		log_print("\'{\' expected.\n");
		exit(1);
	}

	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);

	entry = get_token();
	while (current_state != SYN_END)
	{
		PERFROM_ACTION(syn_fsm, current_state, error_no, entry);
		if (error_no != 0)
		{
			++current_state;
			continue;
		}
		entry = get_token();
	}

	if (entry->token_type != T_SYMBOL || entry->second_token_type != S_RBRACE)
	{
		log_print("\'}\' expected.\n");
		exit(1);
	}

	fprintf(Output_file_id, "<symbol> %s </symbol>\n", entry->sym_name);

	fprintf(Output_file_id, "</class>\n");

	return 0;
}

int syntax_action()
{
	int current_state = SYN_CLASS;
	int error_no;

	symbol_table_init();

	PERFROM_ACTION(syn_fsm, current_state, error_no, NULL);

	return 0;
}