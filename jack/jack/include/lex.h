#ifndef LEX_H
#define LEX_H

#define LINE_IS_END(x) ((x) == '\0' || (x) == '\n')
#define LINE_NOT_END(x) ((x) != '\0' && (x) != '\n')

typedef enum TOKENTYPE
{
	T_KEYWORD, T_SYMBOL, T_IDENTIFIER, T_INT_CONST, T_STRING_CONST
}TokenType;

typedef enum SECONDTOKENTYPE
{
	A_NULL, 

	/* key word type */
	K_BEGIN, 
	K_CLASS, K_METHOD, K_FUNCTION, K_CONSTRUCTOR, 
	K_INT, K_BOOLEAN, K_CHAR, K_VOID, 
	K_VAR, K_STATIC, K_FIELD,

	K_STATEMENT_BEGIN,
	K_LET, K_DO, K_IF, K_ELSE, K_WHILE, K_RETURN,
	K_STATEMENT_END,

	K_KEYWORDS_CONSTANT_BEGIN,
	K_TRUE, K_FALSE, K_NULL, 
	K_THIS,
	K_KEYWORDS_CONSTANT_END,

	K_END,

	/* symbol type */
	S_BEGIN,
	S_LPARENT, S_RPARENT,            /* () */
	S_LBRACKET, S_RBRACKET,          /* [] */
	S_LBRACE, S_RBRACE,              /* {} */

	S_COMMA,                         /* , */
	S_SEMICOLON,                     /* ; */
	S_DOT,                           /* . */
	S_OP_BEGIN,
	S_ADD,                           /* + */
	S_SUB,                           /* - */
	S_MUL,                           /* * */
	S_DIV,                           /* / */
	S_AND,                           /* & */
	S_OR,                            /* | */
	S_BIG,                           /* > */
	S_SMALL,                         /* < */
	S_EQUAL_SIGN,                    /* = */
	S_OP_END,
	S_NOT,                           /* ~ */
	S_NEG,                           /* - */

	S_END
}SecondTokenType;

#define MAX_SYMBOL_NAME_LEN 100

struct symbol_table_entry
{
	struct DListNode link;
	char sym_name[MAX_SYMBOL_NAME_LEN + 1];
	TokenType token_type;
	SecondTokenType second_token_type;
};

struct symbol_table_entry * get_token();

#endif