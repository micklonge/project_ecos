/*
 ***************************************************************
 *
 *         FILE NAME      : end.cpp
 *         ORIGINATOR     : ÕıÓ»—Ô(micklongen)
 *         DATE OF ORIGIN : 2009-4-26
 *
 ***************************************************************
 */

#include "stdafx.h"

#include <string.h>

#include "..\..\common\inc\list.h"
#include "..\..\common\inc\hash.h"
#include "..\..\common\inc\log.h"
#include "include\hack.h"
#include "include\end.h"
#include "include\front.h"
#include "include\symbol.h"

int init_insn_struct(struct Instruction *insn)
{
	insn->CMD_type = NULL_COMMAND;
	insn->comp.comp_op.op_type = OPER_NULL;
	insn->comp.comp_op.dest_type = DEST_NULL;
	insn->comp.comp_op.jump_type = JUMP_NULL;

	return 0;
}
int gen_dest(struct Instruction *insn)
{
	return insn->comp.comp_op.dest_type;
}

int gen_comp(struct Instruction *insn)
{
	int code = 0;
	if (SINGLE_OP(insn->comp.comp_op.op_type))
	{
		switch (insn->comp.comp_op.src1)
		{
		case OPED_ZERO:
			code |= 0x0A80;
			break;
		case OPED_ONE:
			if (insn->comp.comp_op.op_type == OPER_POSITIVE)
				code |= 0x0FC0;
			else if (insn->comp.comp_op.op_type == OPER_NEGATIVE)
				code |= 0x0E80;
			break;
		case OPED_REG_D:
			if (insn->comp.comp_op.op_type == OPER_POSITIVE)
				code |= 0x0300;
			else if (insn->comp.comp_op.op_type == OPER_NEGATIVE)
				code |= 0x03C0;
			else if (insn->comp.comp_op.op_type == OPER_NOT)
				code |= 0x0340;
			break;
		case OPED_REG_M:
			code |= COMP_M;
		case OPED_REG_A:
			if (insn->comp.comp_op.op_type == OPER_POSITIVE)
				code |= 0x0C00;
			else if (insn->comp.comp_op.op_type == OPER_NEGATIVE)
				code |= 0x0CC0;
			else if (insn->comp.comp_op.op_type == OPER_NOT)
				code |= 0x0C40;
			break;
		}
	}
	else if (DOUBLE_OP(insn->comp.comp_op.op_type))
	{
		switch (insn->comp.comp_op.op_type)
		{
		case OPER_ADD:
			if (insn->comp.comp_op.src1 == OPED_REG_M || insn->comp.comp_op.src2 == OPED_REG_M)
				code |= COMP_M;
			if (insn->comp.comp_op.src1 == OPED_ONE || insn->comp.comp_op.src2 == OPED_ONE)
			{
				if (insn->comp.comp_op.src1 == OPED_REG_D || insn->comp.comp_op.src2 == OPED_REG_D )
					code |= 0x07C0;
				else 
					code |= 0x0DC0;
				break;
			}
			code |= 0x0080;
			break;
		case OPER_SUB:
			if (insn->comp.comp_op.src2 == OPED_ONE)
			{
				if (insn->comp.comp_op.src1 == OPED_REG_D)
					code |= 0x0380;
				else 
					code |= 0x0C80;
				if (insn->comp.comp_op.src1 == OPED_REG_M)
					code |= COMP_M;
				break;
			}
			else if (insn->comp.comp_op.src1 == OPED_REG_D)
				code |= 0x04C0;
			else
				code |= 0x01C0;
			if (insn->comp.comp_op.src1 == OPED_REG_M || insn->comp.comp_op.src2 == OPED_REG_M)
				code |= COMP_M;
			break;
		case OPER_AND:
			code |= 0x0000;
			if (insn->comp.comp_op.src1 == OPED_REG_M || insn->comp.comp_op.src2 == OPED_REG_M)
				code |= COMP_M;
			break;
		case OPER_OR:
			code |= 0x0540;
			if (insn->comp.comp_op.src1 == OPED_REG_M || insn->comp.comp_op.src2 == OPED_REG_M)
				code |= COMP_M;
			break;
		}
	}
	else 
	{
		log_print("The error operator type.\n");
		exit(1);
	}

	return code;
}

int gen_jump(struct Instruction *insn)
{
	return insn->comp.comp_op.jump_type;
}

int gen_code(struct Instruction *insn)
{
	int code = 0;

	switch(insn->CMD_type)
	{
	case A_COMMAND:
		code = insn->comp.addr;
		code |= PRE_CMD_A;
		out_code(code);
		break;
	case C_COMMAND:
		code |= gen_jump(insn);
		code |= gen_dest(insn);
		code |= gen_comp(insn);
		code |= PRE_CMD_C;
		out_code(code);
		break;
	default:
		log_print("The bad command type.\n");
		exit(1);
		break;
	}

	return 0;
}

int out_code(int code)
{
	int i;
	int flag = 0x8000;
	char insnstr[CODE_LEN + 1];

	for (i = 0; i < 16; ++i)
	{
		if (code & flag)
			insnstr[i] = '1';
		else
			insnstr[i] = '0';
		flag >>= 1;
	}
	insnstr[i] = '\0';

	fprintf(Output_file_id, "%s\n", insnstr);

	return 0;
}