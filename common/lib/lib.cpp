#include "stdafx.h"

#include <stdlib.h>

#include "..\inc\log.h"
#include "..\inc\lib.h"
#include "..\inc\min_malloc.h"

#pragma warning (disable:4313)                                   // ÆÁ±Î¾¯¸æ
#pragma warning ( disable:4311)
#pragma warning ( disable:4312)

void * com_malloc_log(char *filename, char *func, int line, int size)
{
	void *p = NULL;

	log_print("\n%s:%s(%d):", filename, func, line);
	p = min_malloc(size);
	if (p == NULL)
	{
		log_print("There is no enough memory.\n");
		exit(1);
	}
	log_print("Addr: %x, Size: %d\n", p, size);

	return p;
}

void * com_realloc_log(char *filename, char *func, int line, void * buffer, int size)
{
	void *p = NULL;

	log_print("\n%s:%s(%d):", filename, func, line);
	p = (buffer == NULL ? malloc(size) : realloc(buffer, size));
	if (p == NULL)
	{
		log_print("There is no enough memroy.\n");
		exit(1);
	};
	log_print("Addr: %x, Size: %d\n", p, size);

	return p;
}

void * com_free_log(char *filename, char *func, int line, void *addr)
{
	log_print("\n%s:%s(%d): Addr: %x", filename, func, line, (unsigned long)addr);
	min_free(addr);
	return NULL;
}