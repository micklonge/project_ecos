#ifndef GENCODE_H
#define GENCODE_H

/*-----------------------------------------------------------------------------------*
 *                     command information                                           *
 *-----------------------------------------------------------------------------------*/
#define MAX_JVM_CMD_LEN 10
typedef enum
{
	CMD_PUSH, CMD_POP,
	CMD_ARI_BEGIN, CMD_ADD, CMD_SUB, CMD_NEG, CMD_EQ, CMD_GT, CMD_LT, CMD_AND, CMD_OR, CMD_NOT, CMD_ARI_END, 
	CMD_LABEL, CMD_GOTO, CMD_IF_GOTO,
	CMD_FUNCTION, CMD_CALL, CMD_RETURN
}CMDTYPE;

struct CMD_INFO
{
	char name[MAX_JVM_CMD_LEN + 1];
	CMDTYPE type;
	int (*handler)(char *);
	char arith_sym[10];
};

extern int num_of_cmd;
extern struct CMD_INFO cmd_info[];

/*-----------------------------------------------------------------------------------*
 *                     segment information                                           *
 *-----------------------------------------------------------------------------------*/
#define SEGMENT_NAME_LEN     10
typedef enum
{
	SEG_BEGIN, SEG_ARGUMENT, SEG_LOCAL, SEG_STATIC, SEG_CONSTANT, SEG_THIS, SEG_THAT, SEG_POINTER, SEG_TEMP, SEG_END
}SEGMENT_TYPE;

struct SEGMENT_INFO
{
	char name[SEGMENT_NAME_LEN + 1];
	SEGMENT_TYPE type;
	char addr[6];
};

extern int num_of_segment;
extern struct SEGMENT_INFO seg_info[];

#endif