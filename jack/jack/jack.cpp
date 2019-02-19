// jack.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "..\..\common\inc\common.h"
#include "..\..\common\inc\log.h"
#include "..\..\common\inc\list.h"
#include "..\..\common\inc\hash.h"
#include "..\..\common\inc\lib.h"
#include "include\jack.h"
#include "include\lex.h"
#include "include\symbol.h"
#include "include\syntax.h"

#pragma warning (disable:4996)                                   // 屏蔽警告

char input_file_name[MAX_FILE_NAME + 1];
char output_file_name[MAX_FILE_NAME + 1];

const char* suffix_input_file_name = ".jack";
const char* suffix_output_file_name = ".xml";
#ifdef LOG_ENABLE
const char* Log_file_name = "jack.log";
#endif

FILE* Input_file_id = NULL;
FILE* Output_file_id = NULL;

void Usage()
{
	printf("Usage: jack  -i file1.jack file2.jack ...\n");
}

//char tmp[][100] = {"Main.jack", "Square.jack", "SquareGame.jack"};

int main(int argv, char* argc[])
{
	int i;

	open_log_file;

	if((argv <= 2) || (strcmp(argc[1], "-i") != 0))
	{
		Usage();
		exit(1);
	}

	for (i = 2; i < argv; ++i)
	//for (i = 0; i < 3; ++i)
	{
		strcpy(input_file_name, argc[i]);
		//strcpy(input_file_name, tmp[i]);
		if (strlen(input_file_name) < strlen(suffix_input_file_name) || 
			strcmp(&input_file_name[strlen(input_file_name) - strlen(suffix_input_file_name)], suffix_input_file_name) != 0)
		{
			printf("Error input_file name format!\n");
			exit(1);
		}

		strcpy(output_file_name, input_file_name);
	    strcpy(&output_file_name[strlen(input_file_name) - strlen(suffix_input_file_name)], suffix_output_file_name);

		if (Input_file_id != NULL)
		{
			fclose(Input_file_id);
		}
		if((Input_file_id = fopen(input_file_name, "r")) == NULL)
		{
			printf("Can't open file %s:\n", input_file_name);
			exit(1);
		}

		if (Output_file_id != NULL)
		{
			fclose(Output_file_id);
		}
		if((Output_file_id = fopen(output_file_name, "w")) == NULL)
		{
			printf("Can't open file %s:\n", output_file_name);
			exit(1);
		}

		log_print("\n\nInput file name:%s\n", input_file_name);
		log_print("Output file name:%s\n", output_file_name);

		syntax_action();

		symbol_table_free();
	}

	if (Input_file_id != NULL)
		fclose(Input_file_id);
	if (Output_file_id != NULL)
		fclose(Output_file_id);
	close_log_file;
	return 0;
}
