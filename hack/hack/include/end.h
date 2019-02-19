/*
 ***************************************************************
 *
 *         FILE NAME      : end.h
 *         ORIGINATOR     : ÕıÓ»—Ô(micklongen)
 *         DATE OF ORIGIN : 2009-4-26
 *
 ***************************************************************
 */

#ifndef END_H
#define END_H

#define CODE_LEN        16
#define CODE_NAME       100

#define JUMP_NULL       0x0000
#define JUMP_JGT        0x0001
#define JUMP_JEQ        0x0002
#define JUMP_JGE        0x0003
#define JUMP_JLT        0x0004
#define JUMP_JNE        0x0005
#define JUMP_JLE        0x0006
#define JUMP_JMP        0x0007

#define DEST_NULL       0x0000
#define DEST_M          0x0008
#define DEST_D          0x0010
#define DEST_A          0x0020

#define COMP_M          0x1000
#define COMP_D          0x0800
#define COMP_A          0x0200

#define PRE_CMD_A       0x0000
#define PRE_CMD_C       0xE000

#define IS_REG_A(x)     (((x) == 'a') || ((x) == 'A'))
#define IS_REG_M(x)     (((x) == 'm') || ((x) == 'M'))
#define IS_REG_D(x)     (((x) == 'd') || ((x) == 'D'))

#define IS_OPND_JGT(x)  (strcmp(x, "JGT") == 0)
#define IS_OPND_JEQ(x)  (strcmp(x, "JEQ") == 0)
#define IS_OPND_JGE(x)  (strcmp(x, "JGE") == 0)
#define IS_OPND_JLT(x)  (strcmp(x, "JLT") == 0)
#define IS_OPND_JNE(x)  (strcmp(x, "JNE") == 0)
#define IS_OPND_JLE(x)  (strcmp(x, "JLE") == 0)
#define IS_OPND_JMP(x)  (strcmp(x, "JMP") == 0)

#define SINGLE_OP(x)    (((x) == OPER_POSITIVE) || ((x) == OPER_NEGATIVE) || ((x) == OPER_NOT))
#define DOUBLE_OP(x)    (((x) == OPER_ADD) || ((x) == OPER_SUB) || ((x) == OPER_AND) || ((x) == OPER_OR))

typedef enum
{
	NULL_COMMAND, A_COMMAND, C_COMMAND, L_COMMAND
}Command_type;

typedef enum
{
	OPER_NULL, OPER_POSITIVE, OPER_NEGATIVE, OPER_NOT, OPER_ADD, OPER_SUB, OPER_AND, OPER_OR
}Operactor_type;

typedef enum
{
	OPED_NULL, OPED_ZERO, OPED_ONE, OPED_REG_A, OPED_REG_D, OPED_REG_M
}Operand_type;

struct COMP_op
{
	Operactor_type op_type;
	Operand_type src1;
	Operand_type src2;
	int jump_type;
	int dest_type;
};

struct Instruction
{
	Command_type CMD_type;
	union
	{
		struct COMP_op comp_op;
		int addr;
	}comp;
};

int init_insn_struct(struct Instruction *insn);
int gen_dest(struct Instruction *insn);
int gen_comp(struct Instruction *insn);
int gen_jump(struct Instruction *insn);
int out_code(int code);
int gen_code(struct Instruction *insn);
#endif