#include "stdafx.h"

#include "..\..\common\inc\log.h"

#include "include\jvm.h"
#include "include\gencode.h"

/*-----------------------------------------------------------------------------------*
 *                     function protype                                              *
 *-----------------------------------------------------------------------------------*/
static int hvm_push(char *cmdsrc);
static int hvm_pop(char *cmdsrc);
static int hvm_arithmetic(char *cmdsrc);
static int hvm_control_flow(char *cmdsrc);
static int hvm_call(char *cmdsrc);
static int hvm_function(char *cmdsrc);
static int hvm_return(char *cmdsrc);

/*-----------------------------------------------------------------------------------*
 *                     command information                                           *
 *-----------------------------------------------------------------------------------*/
#define SINGLE_ARI(x) ((x) == CMD_NEG || (x) == CMD_NOT)
#define DOUBLE_ARI(x) (!SINGLE_ARI(x))

struct CMD_INFO cmd_info[] =
{
	{"push", CMD_PUSH, hvm_push, ""},
	{"pop", CMD_POP, hvm_pop, ""},
	{"add", CMD_ADD, hvm_arithmetic, "+"},
	{"sub", CMD_SUB, hvm_arithmetic, "-"},
	{"neg", CMD_NEG, hvm_arithmetic, "-"},
	{"eq", CMD_EQ, hvm_arithmetic, "JEQ"},
	{"gt", CMD_GT, hvm_arithmetic, "JGT"},
	{"lt", CMD_LT, hvm_arithmetic, "JLT"},
	{"and", CMD_AND, hvm_arithmetic, "&"},
	{"or", CMD_OR, hvm_arithmetic, "|"},
	{"not", CMD_NOT, hvm_arithmetic, "!"},
	{"label", CMD_LABEL, hvm_control_flow, ""},
	{"goto", CMD_GOTO, hvm_control_flow, ""},
	{"if-goto", CMD_IF_GOTO, hvm_control_flow, ""},
	{"function", CMD_FUNCTION, hvm_function, ""},
	{"call", CMD_CALL, hvm_call, ""},
	{"return", CMD_RETURN, hvm_return, ""}
};
int num_of_cmd = sizeof(cmd_info) / sizeof(struct CMD_INFO);

/*-----------------------------------------------------------------------------------*
 *                     segment information                                           *
 *-----------------------------------------------------------------------------------*/
struct SEGMENT_INFO seg_info[] = 
{
	{"local", SEG_LOCAL, "1"},
	{"argument", SEG_ARGUMENT, "2"},
	{"pointer", SEG_POINTER, "3"},
	{"this", SEG_THIS, "3"},
	{"that", SEG_THAT, "4"},
	{"temp", SEG_TEMP, "5"},
	{"static", SEG_STATIC, "16"},
	{"constant", SEG_CONSTANT, "-1"}
};
int num_of_segment = sizeof(seg_info) / sizeof(struct SEGMENT_INFO);

/*-----------------------------------------------------------------------------------*
 *                     jcond label                                                   *
 *-----------------------------------------------------------------------------------*/
#define MAX_LABEL_LEN 100

#define JCOND_LABEL_0       0
#define JCOND_LABEL_1       1
#define RETURN_LABEL        2

struct Jcond_label
{
       char name[MAX_LABEL_LEN + 1];
       int  count;
};

struct Jcond_label jcond_label[] = 
{
	{"Jcond_lable_0", 0},
	{"Jcond_lable_1", 0},
	{"Return_addr", 0}
};

/*-----------------------------------------------------------------------------------*
 *                     function info                                                 *
 *-----------------------------------------------------------------------------------*/
#define MAX_FUNCNAME_LEN 100

struct function_info
{
	char name[MAX_FUNCNAME_LEN + 1];
	char num_of_argument[10];
};

static struct function_info func_info = {"main", 0};


/*************************************************************************************
 *                                                                                   *
 *                             hvm command                                           *
 *                                                                                   *
 *************************************************************************************/

/*-----------------------------------------------------------------------------------*
 *                     segment                                                       *
 *-----------------------------------------------------------------------------------*/
static SEGMENT_TYPE seg_deal(char *seg, char *index)
{
	int i;

	for (i = 0; i < num_of_segment; ++i)
	{
		if (strcmp(seg, seg_info[i].name) == 0)
			break;
	}
	if (i == num_of_segment)
	{
		log_print("The segment %s don't exits.\n", seg);
		exit(1);
	}
	if ((seg_info[i].type <= SEG_BEGIN) || (seg_info[i].type >= SEG_END))
	{
		log_print("The segment: %s don't exist.\n", seg);
		exit(1);
	}

	if (seg_info[i].type == SEG_CONSTANT)
	{
		fprintf(Output_file_id, "@%s\n",index);
	}
	else if (seg_info[i].type == SEG_STATIC)
	{
		fprintf(Output_file_id, "@%s.%s\n", input_file_name, index);
	}
	else 
	{
		fprintf(Output_file_id, "@%s\n",seg_info[i].addr);
		if (seg_info[i].type == SEG_POINTER || seg_info[i].type == SEG_TEMP)
			fprintf(Output_file_id, "D=A\n");
		else
			fprintf(Output_file_id, "D=M\n");
		fprintf(Output_file_id, "@%s\n",index);
		fprintf(Output_file_id, "A=D+A\n");
	}

	return seg_info[i].type;
}

/*-----------------------------------------------------------------------------------*
 *                     push and pop                                                  *
 *-----------------------------------------------------------------------------------*/

static int store_value_on_stack()
{
	/* push value on stack */
    fprintf(Output_file_id, "@0\n");
    fprintf(Output_file_id, "A=M\n");
    fprintf(Output_file_id, "M=D\n");
    /* sp=sp+1 */
    fprintf(Output_file_id, "@0\n");
    fprintf(Output_file_id, "M=M+1\n");

	return 0;
}

static int get_value_from_stack()
{
	/* sp=sp-1 */
    fprintf(Output_file_id, "@0\n");
    fprintf(Output_file_id, "M=M-1\n");
    /* pop a value from stack */
    fprintf(Output_file_id, "A=M\n");
    fprintf(Output_file_id, "D=M\n");

	return 0;
}

static int hvm_push(char *cmdsrc)
{
	char cmd[MAX_JVM_CMD_LEN + 1], seg[SEGMENT_NAME_LEN + 1], index[10];

	if (sscanf(cmdsrc, "%s %s %s", cmd, seg, index) != 3)
	{
		log_print("The push format is wrong.\n");
		exit(1);
	}
	if (strcmp(cmd, "push") != 0)
	{
		log_print("The hvm command is not push.\n");
		exit(1);
	}

	if (seg_deal(seg, index) == SEG_CONSTANT)
		fprintf(Output_file_id, "D=A\n");
	else
		fprintf(Output_file_id, "D=M\n");
	store_value_on_stack();

	return 0;
}

static int hvm_pop(char *cmdsrc)
{
	char cmd[MAX_JVM_CMD_LEN + 1], seg[SEGMENT_NAME_LEN + 1], index[10];

	if (sscanf(cmdsrc, "%s %s %s", cmd, seg, index) != 3)
	{
		log_print("The pop format is wrong.\n");
		exit(1);
	}
	if (strcmp(cmd, "pop") != 0)
	{
		log_print("The hvm command is not pop.\n");
		exit(1);
	}

	seg_deal(seg, index);

	fprintf(Output_file_id, "D=A\n");
    fprintf(Output_file_id, "@R13\n");
	fprintf(Output_file_id, "M=D\n");
	get_value_from_stack();
	fprintf(Output_file_id, "@R13\n");
	fprintf(Output_file_id, "A=M\n");
	fprintf(Output_file_id, "M=D\n");

	return 0;
}

/*-----------------------------------------------------------------------------------*
 *                     arithmetic                                                    *
 *-----------------------------------------------------------------------------------*/
static int hvm_arithmetic(char *cmdsrc)
{
	int i;
	char cmd[MAX_JVM_CMD_LEN + 1];

	if (sscanf(cmdsrc, "%s", cmd) != 1)
	{
		log_print("The arithmetic format is wrong.\n");
		exit(1);
	}
	for (i = 0; i < num_of_cmd; ++i)
	{
		if (strcmp(cmd, cmd_info[i].name) == 0)
			break;
	}
	if ((i == num_of_cmd) || (cmd_info[i].type <= CMD_ARI_BEGIN) || (cmd_info[i].type >= CMD_ARI_END))
	{
		log_print("The arithmetic command is not right: %s\n", cmd);
		exit(1);
	}

	if (SINGLE_ARI(cmd_info[i].type))
	{
		get_value_from_stack();
		fprintf(Output_file_id, "D=%sD\n", cmd_info[i].arith_sym);
		store_value_on_stack();
	}
	else
	{
		get_value_from_stack();
		fprintf(Output_file_id, "@R13\n");
		fprintf(Output_file_id, "M=D\n");
		get_value_from_stack();
		switch(cmd_info[i].type)
		{
		case CMD_ADD:
		case CMD_SUB:
		case CMD_AND:
		case CMD_OR:
			fprintf(Output_file_id, "@R13\n");
			fprintf(Output_file_id, "D=D%sM\n", cmd_info[i].arith_sym);
			break;
		case CMD_EQ:
		case CMD_GT:
		case CMD_LT:
			fprintf(Output_file_id, "@R13\n");
			fprintf(Output_file_id, "D=D-M\n");
			fprintf(Output_file_id, "@%s.%d\n",jcond_label[JCOND_LABEL_0].name, jcond_label[JCOND_LABEL_0].count);
			fprintf(Output_file_id, "D;%s\n", cmd_info[i].arith_sym);
			fprintf(Output_file_id, "D=0\n");
			fprintf(Output_file_id, "@%s.%d\n",jcond_label[JCOND_LABEL_1].name, jcond_label[JCOND_LABEL_1].count);
			fprintf(Output_file_id, "0;JMP\n");
			fprintf(Output_file_id, "(%s.%d)\n",jcond_label[JCOND_LABEL_0].name, jcond_label[JCOND_LABEL_0].count);
			jcond_label[JCOND_LABEL_0].count++;
			fprintf(Output_file_id, "D=-1\n");
			fprintf(Output_file_id, "(%s.%d)\n",jcond_label[JCOND_LABEL_1].name, jcond_label[JCOND_LABEL_1].count);
			jcond_label[JCOND_LABEL_1].count++;  
			break;
		default:
			break;
		}
		store_value_on_stack();    
	}

	return 0;
}

/*-----------------------------------------------------------------------------------*
 *                     function gen                                                  *
 *-----------------------------------------------------------------------------------*/
static int hvm_call(char *cmdsrc)
{
	int i;
	char cmd[MAX_JVM_CMD_LEN + 1], tmp_function_name[MAX_FUNCNAME_LEN + 1], tmp_num_of_argument[10];

	if (sscanf(cmdsrc, "%s %s %s", cmd, tmp_function_name, tmp_num_of_argument) != 3)
	{
		log_print("The call format is wrong.\n");
		exit(1);
	}

	for (i = 0; i < num_of_cmd; ++i)
	{
		if (strcmp(cmd, cmd_info[i].name) == 0)
			break;
	}
	if (i == num_of_cmd || cmd_info[i].type != CMD_CALL)
	{
		log_print("The function command is not right: %s\n", cmd);
		exit(1);
	}

	fprintf(Output_file_id, "@%s$%s.%d\n",func_info.name, tmp_function_name, jcond_label[RETURN_LABEL].count);
	fprintf(Output_file_id, "D=A\n");
	store_value_on_stack();

	/*push LCL*/
	fprintf(Output_file_id, "@1\n");
	fprintf(Output_file_id, "D=M\n");
	store_value_on_stack();

	/*push ARG*/
	fprintf(Output_file_id, "@2\n");
	fprintf(Output_file_id, "D=M\n");
	store_value_on_stack();

	/*push THIS*/
	fprintf(Output_file_id, "@3\n");
	fprintf(Output_file_id, "D=M\n");
	store_value_on_stack();

	/*push THAT*/
	fprintf(Output_file_id, "@4\n");
	fprintf(Output_file_id, "D=M\n");
	store_value_on_stack();

	/*ARG = SP - n - 5*/
	fprintf(Output_file_id, "@%s\n",tmp_num_of_argument);
	fprintf(Output_file_id, "D=A\n");
	fprintf(Output_file_id, "@0\n");
	fprintf(Output_file_id, "D=M-D\n");
	fprintf(Output_file_id, "@5\n");
	fprintf(Output_file_id, "D=D-A\n");
	fprintf(Output_file_id, "@2\n");
	fprintf(Output_file_id, "M=D\n");

	/*LCL = SP*/
	fprintf(Output_file_id, "@0\n");
	fprintf(Output_file_id, "D=M\n");
	fprintf(Output_file_id, "@1\n");
	fprintf(Output_file_id, "M=D\n");

	/*goto funcname*/
	fprintf(Output_file_id, "@%s\n",tmp_function_name);
	fprintf(Output_file_id, "0;JMP\n");

	/*(return-name)*/
	fprintf(Output_file_id, "(%s$%s.%d)\n",func_info.name, tmp_function_name, jcond_label[RETURN_LABEL].count);
	jcond_label[RETURN_LABEL].count++;

	return 0;
}

static int hvm_function(char *cmdsrc)
{
	int i;
	char cmd[MAX_JVM_CMD_LEN + 1];

	if (sscanf(cmdsrc, "%s %s %s", cmd, func_info.name, func_info.num_of_argument) != 3)
	{
		log_print("The function format is wrong.\n");
		exit(1);
	}

	for (i = 0; i < num_of_cmd; ++i)
	{
		if (strcmp(cmd, cmd_info[i].name) == 0)
			break;
	}
	if (i == num_of_cmd || cmd_info[i].type != CMD_FUNCTION)
	{
		log_print("The function command is not right: %s\n", cmd);
		exit(1);
	}

	/*(function.name)*/
	fprintf(Output_file_id, "(%s)\n",func_info.name);

	/*repeat k times push 0*/
	fprintf(Output_file_id, "@%s\n",func_info.num_of_argument);
	fprintf(Output_file_id, "D=A\n");
	fprintf(Output_file_id, "@R13\n");
	fprintf(Output_file_id, "M=D\n");
	fprintf(Output_file_id, "(%s.%d)\n",jcond_label[JCOND_LABEL_0].name, jcond_label[JCOND_LABEL_0].count);
	fprintf(Output_file_id, "@R13\n");
	fprintf(Output_file_id, "D=M\n");
	fprintf(Output_file_id, "@%s.%d\n",jcond_label[JCOND_LABEL_1].name, jcond_label[JCOND_LABEL_1].count);
	fprintf(Output_file_id, "D;JEQ\n");
	fprintf(Output_file_id, "D=0\n");
	store_value_on_stack();
	fprintf(Output_file_id, "@R13\n");
	fprintf(Output_file_id, "M=M-1\n");
	fprintf(Output_file_id, "@%s.%d\n",jcond_label[JCOND_LABEL_0].name, jcond_label[JCOND_LABEL_0].count);
	jcond_label[JCOND_LABEL_0].count++;
	fprintf(Output_file_id, "0;JMP\n");
	fprintf(Output_file_id, "(%s.%d)\n",jcond_label[JCOND_LABEL_1].name, jcond_label[JCOND_LABEL_1].count);
	jcond_label[JCOND_LABEL_1].count++;    

	return 0;
}

static int hvm_return(char *cmdsrc)
{
	int i;

	char cmd[MAX_JVM_CMD_LEN + 1];

	if (sscanf(cmdsrc, "%s", cmd) != 1)
	{
		log_print("The return format is wrong.\n");
		exit(1);
	}

	for (i = 0; i < num_of_cmd; ++i)
	{
		if (strcmp(cmd, cmd_info[i].name) == 0)
			break;
	}
	if (i == num_of_cmd || cmd_info[i].type != CMD_RETURN)
	{
		log_print("The return command is not right: %s\n", cmd);
		exit(1);
	}

	/*FRAME = LCL*/
	fprintf(Output_file_id, "@1\n");
	fprintf(Output_file_id, "D=M\n");
	fprintf(Output_file_id, "@R13\n");
	fprintf(Output_file_id, "M=D\n");

	/*RET = *(FRAME - 5)*/
	fprintf(Output_file_id, "@5\n");
	fprintf(Output_file_id, "A=D-A\n");
	fprintf(Output_file_id, "D=M\n");
	fprintf(Output_file_id, "@R14\n");
	fprintf(Output_file_id, "M=D\n");

	/**ARG = pop()*/
	get_value_from_stack();
	fprintf(Output_file_id, "@2\n");
	fprintf(Output_file_id, "A=M\n");
	fprintf(Output_file_id, "M=D\n");

	/*SP = ARG + 1*/
	fprintf(Output_file_id, "@2\n");
	fprintf(Output_file_id, "D=M+1\n");
	fprintf(Output_file_id, "@0\n");
	fprintf(Output_file_id, "M=D\n");

	/*THAT = *(FRAME - 1)*/
	fprintf(Output_file_id, "@R13\n");
	fprintf(Output_file_id, "A=M-1\n");
	fprintf(Output_file_id, "D=M\n");
	fprintf(Output_file_id, "@4\n");
	fprintf(Output_file_id, "M=D\n");

	/*THIS = *(FRAME - 2)*/
	fprintf(Output_file_id, "@R13\n");
	fprintf(Output_file_id, "D=M\n");
	fprintf(Output_file_id, "@2\n"); 
	fprintf(Output_file_id, "A=D-A\n"); 
	fprintf(Output_file_id, "D=M\n");
	fprintf(Output_file_id, "@3\n");
	fprintf(Output_file_id, "M=D\n");

	/*ARG = *(FRAME - 3)*/
	fprintf(Output_file_id, "@R13\n");
	fprintf(Output_file_id, "D=M\n");
	fprintf(Output_file_id, "@3\n"); 
	fprintf(Output_file_id, "A=D-A\n"); 
	fprintf(Output_file_id, "D=M\n");
	fprintf(Output_file_id, "@2\n");
	fprintf(Output_file_id, "M=D\n");

	/*LCL = *(FRAME - 4)*/
	fprintf(Output_file_id, "@R13\n");
	fprintf(Output_file_id, "D=M\n");
	fprintf(Output_file_id, "@4\n"); 
	fprintf(Output_file_id, "A=D-A\n"); 
	fprintf(Output_file_id, "D=M\n");
	fprintf(Output_file_id, "@1\n");
	fprintf(Output_file_id, "M=D\n");

	/*goto RET*/
	fprintf(Output_file_id, "@R14\n");
	fprintf(Output_file_id, "A=M\n");
	fprintf(Output_file_id, "0;JMP\n");

	return 0;
}

/*-----------------------------------------------------------------------------------*
 *                     program flow                                                  *
 *-----------------------------------------------------------------------------------*/
static int hvm_control_flow(char *cmdsrc)
{
	int i;
	char cmd[MAX_JVM_CMD_LEN + 1], cmd_label[MAX_LABEL_LEN + 1];

	if (sscanf(cmdsrc, "%s %s", cmd, cmd_label) != 2)
	{
		log_print("The control flow format is wrong.\n");
		exit(1);
	}
	for (i = 0; i < num_of_cmd; ++i)
	{
		if (strcmp(cmd, cmd_info[i].name) == 0)
			break;
	}
	if (i == num_of_cmd)
	{
		log_print("The control flow command is not right: %s\n", cmd);
		exit(1);
	}

	switch(cmd_info[i].type)
	{
	case CMD_LABEL:
		fprintf(Output_file_id, "(%s$%s)\n", func_info.name, cmd_label);
		break;
	case CMD_GOTO:
		fprintf(Output_file_id, "@%s$%s\n", func_info.name, cmd_label);
        fprintf(Output_file_id, "0;JMP\n");
		break;
	case CMD_IF_GOTO:
		get_value_from_stack();
        fprintf(Output_file_id, "@%s$%s\n", func_info.name, cmd_label);
        fprintf(Output_file_id, "D;JNE\n");
		break;
	default:
		log_print("The wrong command %s.\n", cmd_info[i].name);
		exit(1);
		break;
	}

	return 0;
}