/*
 ***************************************************************
 *
 *         FILE NAME      : front.h
 *         ORIGINATOR     : ÕıÓ»—Ô(micklongen)
 *         DATE OF ORIGIN : 2009-4-26
 *
 ***************************************************************
 */

#ifndef FRONT_H
#define FRONT_H

typedef enum
{
	START_FSM, END_FSM, 
	PASS0_FSM, 
	    PASS0_A_COMMAND_FSM, PASS0_C_COMMAND_FSM, PASS0_L_COMMAND_FSM,
	PASS1_FSM, 
	    PASS1_A_COMMAND_FSM, PASS1_C_COMMAND_FSM, PASS1_L_COMMAND_FSM
}State;

extern int code_addr;
extern int data_addr;

extern struct DListHeader Toke_head;

char * read_line();
char * get_variable(char **);
int expression_oper(char **p, char **line, int num, struct Instruction *insn);
int expression_opnd(char **p, char **line, int num, struct Instruction *insn);
int get_insn_struct(char **, struct Instruction *);
int parser(char *);

#endif