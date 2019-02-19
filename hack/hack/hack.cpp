// hack.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

/*
 *******************************************************************
 *
 *         FILE NAME      : hack.cpp
 *         ORIGINATOR     : 王钊扬(micklongen)
 *         DATE OF ORIGIN : 
 *
 *******************************************************************
 */

#include <stdio.h>
#include <string.h>

#include "..\..\common\inc\common.h"
#include "..\..\common\inc\log.h"
#include "..\..\common\inc\list.h"
#include "..\..\common\inc\hash.h"
#include "..\..\common\inc\lib.h"
#include "include\hack.h"
#include "include\symbol.h"
#include "include\front.h"

/*
 *******************************************************************
 * global variable
 *******************************************************************
 */
const char* suffix_input_file_name = ".asm";
const char* suffix_output_file_name = ".hack";
#ifdef LOG_ENABLE
const char* Log_file_name = "hack.log";
#endif

int pass;

FILE* Input_file_id;
FILE* Output_file_id;

int code_addr;
int data_addr;

void Usage()
{
	printf("Usage: hack infile\n");
}

int main(int argv, char* argc[])
{
	char *line;
	char input_file_name[MAX_FILE_NAME + 1];
	char output_file_name[MAX_FILE_NAME + 1];

	if (argv != 2)
	{
		Usage();
		return 0;
	}

	strcpy(input_file_name, argc[1]);
	if (strlen(input_file_name) < strlen(suffix_input_file_name) || 
		strcmp(&input_file_name[strlen(input_file_name) - strlen(suffix_input_file_name)], suffix_input_file_name) != 0)
	{
		printf("Error input_file name format!\n");
		return 0;
	}

	strcpy(output_file_name, input_file_name);
	strcpy(&output_file_name[strlen(input_file_name) - strlen(suffix_input_file_name)], suffix_output_file_name);

	open_log_file;
	if((Input_file_id = fopen(input_file_name, "r")) == NULL)
	{
		log_print("Can't open file %s:\n", input_file_name);
		close_log_file;
		return 0;
	}
	if((Output_file_id = fopen(output_file_name, "w")) == NULL)
	{
		log_print("Can't open file %s:\n", output_file_name);
		fclose(Input_file_id);
		close_log_file;
		return 0;
	}

	log_print("Input file name:%s\n", input_file_name);
	log_print("Output file name:%s\n", output_file_name);
	log_print("All the files we need are open.\n\n");

	symbol_init();
	log_print("symbol_init completed.\n\n");

	/* two-pass assembler */
	for (pass = 0; pass < 2; pass++)
	{
		log_print("\n---------------------------------------------- The %s pass ----------------------------------------------\n\n", pass == 0 ? "first" : "second");
		rewind(Input_file_id);
		code_addr = START_CODE_ADDR;
	    data_addr = START_DATA_ADDR;

		while((line = read_line()) != NULL)
		{
			parser(line);
			code_addr++;
			com_free(line);
		}
	}

	symbol_table_free();

	fclose(Input_file_id);
	fclose(Output_file_id);
	close_log_file;

	return 0;
}

