/*
 *******************************************************************
 *
 *         FILE NAME      : front.cpp
 *         ORIGINATOR     : ÕıÓ»—Ô(micklongen)
 *         DATE OF ORIGIN : 2009-4-26
 *
 *******************************************************************
 */

#include "stdafx.h"

#include <string.h>
#include <ctype.h>

#include "..\..\common\inc\common.h"
#include "..\..\common\inc\log.h"
#include "..\..\common\inc\list.h"
#include "..\..\common\inc\hash.h"
#include "..\..\common\inc\lib.h"
#include "include\hack.h"
#include "include\symbol.h"
#include "include\front.h"
#include "include\end.h"

/*
 -------------------------------------------------------------------
 *  get words
 -------------------------------------------------------------------
 */
#define MAX_BUFFER_INC 1000

static int code_start_addr;
static int data_start_addr;

struct DListHeader Toke_head;

#define is_variable_start(ch) (isalpha(ch) || ch == '_' || ch == '.' || ch == '$' || ch == ':')
#define is_variable(ch) (is_variable_start(ch) || isdigit(ch))

typedef enum
{
	STAT_NULL, STAT_START, STAT_DEST, STAT_COMP, STAT_JUMP, STAT_END
}Code_stat;

/* read a line */
char * read_line()
{
	int buffer_size = 0;
	int offset;
	char *buffer = NULL;
	char *p = NULL;

	do
	{
		if (buffer != NULL)
			com_free(buffer);
		buffer_size = MAX_BUFFER_INC; 
		buffer = (char *)com_malloc(sizeof(char) * buffer_size); 
		p = buffer;
		/* read a line */
		while(1)
		{
			p = fgets(p, buffer_size - (p - buffer), Input_file_id);
			if (p == NULL)
				return NULL;
			p += strlen(p);

			if (p[-1] == '\n')
				break;

			if (buffer_size - strlen(buffer) > 10)
				continue;

			buffer_size += MAX_BUFFER_INC;

			/* realloc */
			offset = p - buffer;
			buffer = (char *)com_realloc(buffer, buffer_size);
			p = buffer + offset;
		}
	}while((buffer[0] == '\n') || (buffer[0] == '/' && buffer[1] == '/'));  /*ignore the comments and space line*/
	
	buffer[strlen(buffer) - 1] = '\0';
	p = buffer + strlen(buffer) - 1;
	while(p > buffer)
	{
		if (*p == '/' && p[1] == '/')
			*p = '\0';
		--p;
	}

	log_print("The contents of read_line: %s\n", buffer);
	return buffer;
}

/* get a variable name */
char * get_variable(char **line)
{
	char *start = NULL, *p = NULL;

	start = *line;
	if (!is_variable_start(**line))
	{
		log_print("The wrong character start.\n");
		exit(1);
	}
	while(is_variable(**line))
	{
		(*line)++;
	}

	p = (char *)com_malloc(sizeof(char) * (*line - start + 1));
	strncpy(p, start, *line - start);
	p[*line - start] = '\0';

	return p;
}

int get_num(char **line)
{
	int sum = 0;

	while (isdigit(**line))
	{
		sum = sum * 10 + **line - '0';
		(*line)++;
	}

	return sum;
}

int expression_oper(char **p, char **line, int num, struct Instruction *insn)
{
	if (*p == *line)
		return 0;

	while(**p == ' ')
		++(*p);

	switch(**p)
	{
	case '+':
		if (num == 0)
			insn->comp.comp_op.op_type = OPER_POSITIVE;
		else 
			insn->comp.comp_op.op_type = OPER_ADD;
		break;
	case '-':
		if (num == 0)
			insn->comp.comp_op.op_type = OPER_NEGATIVE;
		else 
			insn->comp.comp_op.op_type = OPER_SUB;
		break;
	case '!':
		insn->comp.comp_op.op_type = OPER_NOT;
		break;
	case '&':
		insn->comp.comp_op.op_type = OPER_AND;
		break;
	case '|':
		insn->comp.comp_op.op_type = OPER_OR;
		break;
	default:
		insn->comp.comp_op.op_type = OPER_POSITIVE;
		break;
	}

	(*p)++;
	expression_opnd(p, line, num + 1, insn);

	return 0;
}

int expression_opnd(char **p, char **line, int num, struct Instruction *insn)
{
	if (*p == *line)
		return 0;

	while(**p == ' ')
		++(*p);

	if (**p == '0')
	{
		expression_oper(p, line, num, insn);
		insn->comp.comp_op.src1 = OPED_ZERO;
		return 0;
	}
	else if (**p == '1')
	{
		if (num == 0)
		{
			expression_oper(p, line, num, insn);
			insn->comp.comp_op.src1 = OPED_ONE;
		}
		else if (num == 1)
			insn->comp.comp_op.src1 = OPED_ONE;
		else
			insn->comp.comp_op.src2 = OPED_ONE;

		return 0;
	}
	else if (IS_REG_A(**p))
	{
		if (num == 0)
		{
			expression_oper(p, line, num + 1, insn);
			insn->comp.comp_op.src1 = OPED_REG_A;
			(*p)++;
		}
		else if (num == 1)
			insn->comp.comp_op.src1 = OPED_REG_A;
		else
			insn->comp.comp_op.src2 = OPED_REG_A;

		return 0;
	}
	else if (IS_REG_M(**p))
	{
		if (num == 0)
		{
			expression_oper(p, line, num + 1, insn);
			insn->comp.comp_op.src1 = OPED_REG_M;
			(*p)++;
		}
		else if (num == 1)
			insn->comp.comp_op.src1 = OPED_REG_M;
		else
			insn->comp.comp_op.src2 = OPED_REG_M;

		return 0;
	}
	else if (IS_REG_D(**p))
	{
		if (num == 0)
		{
			expression_oper(p, line, num + 1, insn);
			insn->comp.comp_op.src1 = OPED_REG_D;
			(*p)++;
		}
		else if (num == 1)
			insn->comp.comp_op.src1 = OPED_REG_D;
		else
			insn->comp.comp_op.src2 = OPED_REG_D;

		return 0;
	}
	else
	{
		expression_oper(p, line, num, insn);
	}

	return 0;
}

int get_insn_struct(char **line, struct Instruction *insn)
{
	Code_stat curr_stat = STAT_START;
	char *p = *line;

	while (1)
	{
		if (**line == ' ')
		{
			++(*line);
			continue;
		}
		if ((**line != '=') && (**line != ';') && (**line != '\0'))
		{
			++(*line);
			continue;
		}

	    switch (curr_stat)
		{
		case STAT_START: 
			if (**line == '=') 
			{
				curr_stat = STAT_DEST;
				while (p != *line)
				{
					if (*p == ' ')
					{
						++p;
						continue;
					}

					if (IS_REG_A(*p))
						insn->comp.comp_op.dest_type |= DEST_A;
					else if (IS_REG_M(*p))
						insn->comp.comp_op.dest_type |= DEST_M;
					else if (IS_REG_D(*p))
						insn->comp.comp_op.dest_type |= DEST_D;
					++p;
				}
				break;
			}
		case STAT_DEST:
			if (**line == ';' || **line == '\0')
			{
				curr_stat = STAT_COMP;

				expression_opnd(&p, line, 0, insn);

				break;
			}
		case STAT_COMP:
			while (*(*line - 1) == ' ')
			{
				--(*line);
				**line = '\0';
			}
			if (**line == '\0')
			{
				curr_stat = STAT_JUMP; 
				if (IS_OPND_JGT(p))
					insn->comp.comp_op.jump_type = JUMP_JGT;
				else if (IS_OPND_JEQ(p))
					insn->comp.comp_op.jump_type = JUMP_JEQ;
				else if (IS_OPND_JGE(p))
					insn->comp.comp_op.jump_type = JUMP_JGE;
				else if (IS_OPND_JLT(p))
					insn->comp.comp_op.jump_type = JUMP_JLT;
				else if (IS_OPND_JNE(p))
					insn->comp.comp_op.jump_type = JUMP_JNE;
				else if (IS_OPND_JLE(p))
					insn->comp.comp_op.jump_type = JUMP_JLE;
				else if (IS_OPND_JMP(p))
					insn->comp.comp_op.jump_type = JUMP_JMP;
				break;
			}
		case STAT_JUMP:
		default:
			log_print("The bad C_Command stat.\n");
			exit(1);
			break;
		}

		if (**line == '\0')
			break;
		++(*line);
		p = *line;
	}

	return 0;
}

int parser(char *line)
{
	char *p = line;
	char *var_name = NULL;
	State next_state = START_FSM;
	struct symbol_info sym_info;
	struct Instruction insn;
	int address;

	while (*p != NULL)
	{
		if (*p == ' ')
		{
			++p;
			continue;
		}
		if (*p == '/' && p[1] == '/')
			return 1;
		switch(next_state)
		{
		case START_FSM:
			if (pass == 0)
				next_state = PASS0_FSM;
			else if (pass == 1)
				next_state = PASS1_FSM;
			else
			{
				log_print("The wrong pass_fsm.\n");
				exit(1);
			}
			break;
		case END_FSM:
			return 0;
			break;
		case PASS0_FSM:
			if (*p == '@')
			{
				next_state = PASS0_A_COMMAND_FSM;
				++p;
			}
			else if (*p == '(')
			{
				next_state = PASS0_L_COMMAND_FSM;
				++p;
			}
			else
				next_state = PASS0_C_COMMAND_FSM;
			break;
		case PASS0_A_COMMAND_FSM:
			next_state = END_FSM;
			break;
		case PASS0_C_COMMAND_FSM:
			next_state = END_FSM;
			break;
		case PASS0_L_COMMAND_FSM:
			if (is_variable_start(*p))
			{
				var_name = get_variable(&p);
				sym_info.symbol_name = var_name;
				sym_info.addr = code_addr;
				sym_insert(&sym_info);
				com_free(var_name);
			}
			else
			{
				log_print("Bad L command.\n");
				exit(1);
			}
			if (*p != ')')
			{
				log_print("Miss \')\'.\n");
				exit(1);
			}
			code_addr--;
			++p;
			next_state = END_FSM;
			break;
		case PASS1_FSM:
			if (*p == '@')
			{
				next_state = PASS1_A_COMMAND_FSM;
				++p;
			}
			else if (*p == '(')
			{
				next_state = PASS1_L_COMMAND_FSM;
				++p;
			}
			else
				next_state = PASS1_C_COMMAND_FSM;
			break;
		case PASS1_A_COMMAND_FSM:
			if (is_variable_start(*p))
			{
				var_name = get_variable(&p);
				address = get_symbol_addr(var_name);
				com_free(var_name);
			}
			else if (isdigit(*p))
			{
				address = get_num(&p);
			}
			else
			{
				log_print("Bad A command.\n");
				exit(1);
			}
			log_print("The symbol addr %d.\n", address);
			insn.CMD_type = A_COMMAND;
			insn.comp.addr = address;
			gen_code(&insn);
			next_state = END_FSM;
			break;
		case PASS1_C_COMMAND_FSM:
			init_insn_struct(&insn);
			insn.CMD_type = C_COMMAND;
			get_insn_struct(&p, &insn);
			gen_code(&insn);
			next_state = END_FSM;
			break;
		case PASS1_L_COMMAND_FSM:
			insn.CMD_type = L_COMMAND;
			p += strlen(p);
			next_state = END_FSM;
			break;
		default:
			log_print("The wrong fsm.\n");
			exit(1);
		}
	}

	return 0;
}