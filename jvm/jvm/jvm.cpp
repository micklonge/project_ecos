// jvm.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <stdio.h>
#include <string.h>

#include "..\..\common\inc\common.h"
#include "..\..\common\inc\log.h"
#include "..\..\common\inc\list.h"
#include "..\..\common\inc\hash.h"
#include "..\..\common\inc\lib.h"
#include "include\jvm.h"
#include "include\gencode.h"

char input_file_name[MAX_FILE_NAME + 1];
char output_file_name[MAX_FILE_NAME + 1];

const char* suffix_input_file_name = ".vm";
const char* suffix_output_file_name = ".asm";
#ifdef LOG_ENABLE
const char* Log_file_name = "jvm.log";
#endif

FILE* Input_file_id = NULL;
FILE* Output_file_id = NULL;

void Usage()
{
	printf("Usage: jvm  -o outfile.asm -i file1.vm file2.vm ...\n");
}

/*--------------------------------------------------------------------------------*
 *                         read a line                                            *
 *--------------------------------------------------------------------------------*/
#define MAX_BUFFER_INC 1000

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

int input_file()
{
	char *line;

	while ((line = read_line()) != NULL)
	{
		int i;
	    char cmd[MAX_JVM_CMD_LEN + 1];

		sscanf(line, "%s", cmd);
		for (i = 0; i < num_of_cmd; ++i)
		{
			if (strcmp(cmd, cmd_info[i].name) == 0)
				break;
		}
		if (i == num_of_cmd)
		{
			log_print("The hvm command is not right: %s\n", cmd);
			exit(1);
		}
		
		if (cmd_info[i].handler == NULL)
		{
			log_print("The hvm function is NULL: %s\n", cmd);
			exit(1);
		}
		
		cmd_info[i].handler(line);

		com_free(line);
	}

	return 0;
}

int main(int argv, char* argc[])
{
	int i, flag = 0;
	char *p = NULL;

	open_log_file;

	/* command line process */
	flag = 0;
	for (i = 1; i < argv; ++i)
	{
		if (argc[i][0] == '-')
		{
			if (strlen(argc[i]) != 2)
			{
				flag = 0;
				break;
			}
			switch (argc[i][1])
			{
			case 'i':
			case 'I':
				if (flag != 2 || flag == 0)
				{
					Usage();
					exit(1);
				}

				for (++i; i < argv; ++i)
				{
					strcpy(input_file_name, argc[i]);
					if (strlen(input_file_name) < strlen(suffix_input_file_name) || 
						strcmp(&input_file_name[strlen(input_file_name) - strlen(suffix_input_file_name)], suffix_input_file_name) != 0)
					{
						printf("Error input_file name format!\n");
						exit(1);
					}

					if (Input_file_id != NULL)
					{
						fclose(Input_file_id);
					}
					if((Input_file_id = fopen(input_file_name, "r")) == NULL)
					{
						printf("Can't open file %s:\n", input_file_name);
						exit(1);
					}

					log_print("Input file name:%s\n", input_file_name);

					input_file();
				}
				flag |= 1;
				break;
			case 'o':
			case 'O':
				if (flag != 0)
				{
					Usage();
					exit(1);
				}

				++i;
				strcpy(output_file_name, argc[i]);
				if (strlen(output_file_name) < strlen(suffix_output_file_name) || 
					strcmp(&output_file_name[strlen(output_file_name) - strlen(suffix_output_file_name)], suffix_output_file_name) != 0)
				{
					printf("Error output_file name format!\n");
					exit(1);
				}

				if((Output_file_id = fopen(output_file_name, "w")) == NULL)
				{
					printf("Can't open file %s:\n", output_file_name);
					exit(1);
				}

				flag |= 2;

				log_print("Output file name:%s\n", output_file_name);

				break;
			default:
				Usage();
				exit(1);
				break;
			}
		}
	}

	if (flag != 3)
	{
		Usage();
		exit(1);
	}

	if (Input_file_id != NULL)
		fclose(Input_file_id);
	if (Output_file_id != NULL)
		fclose(Output_file_id);
	close_log_file;

	return 0;
}