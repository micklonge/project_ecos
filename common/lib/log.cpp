/*
 ***************************************************************
 *
 *         FILE NAME      : log.cpp
 *         ORIGINATOR     : ÍõîÈÑï(micklongen)
 *         DATE OF ORIGIN : 
 *
 ***************************************************************
 */
#include "stdafx.h"

#include <stdio.h>
#include <stdarg.h>

#include "..\..\common\inc\common.h"
#include "..\inc\log.h"

#pragma warning (disable:4996)                                   // ÆÁ±Î¾¯¸æ

#ifdef LOG_ENABLE

extern const char* Log_file_name;

FILE* Log_file_id;

int open_log_file_func()
{
	if((Log_file_id = fopen(Log_file, "w")) == NULL)
	{
		log_print("Can't open file %s:\n", Log_file);
		return 0;
	}

	return 0;
}

int close_log_file_func()
{
	fclose(Log_file_id);

	return 0;
}

#endif