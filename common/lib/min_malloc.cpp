#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>

#include "..\inc\list.h"
#include "..\inc\min_malloc.h"

#pragma warning ( disable:4311)
#pragma warning ( disable:4312)

typedef unsigned long uint_32;
#ifndef __cplusplus
#define offsetof(s,m) (size_t)&(((s *)0)->m)
#endif
#define container_of(TYPE, MEMBER, ADDR) \
	(size_t)((size_t)ADDR - offsetof(TYPE, MEMBER))

#define MALLOC_SIZE 10000000
#define MALLOC_HEADER sizeof(struct malloc_header)

#define DATA_ALIGN_16(x) (((x) + 0xF) & 0xFFFFFFF0)

static char *malloc_mem = NULL;

static struct DListHeader malloc_free_list;

struct malloc_header
{
	struct DListNode free_list;  // manage the free block
	uint_32 addr;
	int size;
	int pre_size;
};

void * min_malloc(int size)
{
	struct malloc_header * malloc_point = NULL;
	struct malloc_header * malloc_free = NULL;
	struct DListNode *node = NULL;
	void *p = NULL;

	/* 16 align */
	size = DATA_ALIGN_16(size);

	/* first call min_malloc, and apply a large block */
	if (malloc_mem == NULL)
	{
		malloc_mem = (char *)malloc(MALLOC_SIZE);
		malloc_point = (struct malloc_header *)malloc_mem;
		malloc_point->addr = (uint_32)malloc_mem;
		malloc_point->size = MALLOC_SIZE - (uint_32)MALLOC_HEADER;
		malloc_point->pre_size = 0;
		malloc_point->free_list.next = malloc_point->free_list.pre = NULL;

		initialize_Dlist_header(&malloc_free_list);
		DList_add_tail(&malloc_free_list, &malloc_point->free_list);

		malloc_point = (struct malloc_header *)((uint_32)malloc_mem + MALLOC_SIZE - (uint_32)MALLOC_HEADER);
		malloc_point->addr = 0;                   // 0: indicate the last block
		malloc_point->pre_size = MALLOC_SIZE - (uint_32)MALLOC_HEADER;
		malloc_point->size = MALLOC_HEADER;
		malloc_point->free_list.next = malloc_point->free_list.pre = NULL;
	}

	// search the first enough memory
	node = DList_get_head(&malloc_free_list);
	while (node != NULL)
	{
		malloc_point = (struct malloc_header *)container_of(struct malloc_header, free_list, node);
		if (malloc_point->size >= size + (int)MALLOC_HEADER)
			break;
		node = DList_get_next(&malloc_free_list, &malloc_point->free_list);
	}

	if (node == NULL)
		return NULL;

	p = (void *)(malloc_point->addr + MALLOC_HEADER);
	DList_delete_node(&malloc_free_list, &malloc_point->free_list);
	if (malloc_point->size > size + (int)MALLOC_HEADER * 2)
	{
		malloc_free = (struct malloc_header *)(malloc_point->addr + MALLOC_HEADER + size);
		malloc_free->addr = malloc_point->addr + MALLOC_HEADER + size;
		malloc_free->size = malloc_point->size - MALLOC_HEADER - size;
		malloc_free->pre_size = MALLOC_HEADER + size;
		DList_add_tail(&malloc_free_list, &malloc_free->free_list);

		malloc_point->size = MALLOC_HEADER + size;
		malloc_point = (struct malloc_header *)(malloc_free->addr + malloc_free->size);
		malloc_point->pre_size = malloc_free->size;
	}
	
	return p;
}

void min_free(void *addr)
{
	int coalesce_case = 0;
	struct malloc_header * malloc_point = NULL;
	struct malloc_header * malloc_free_pre = NULL;
	struct malloc_header * malloc_free_next = NULL;

	if (addr == NULL)
		return;

	/* find out the management structure */
	malloc_point = (struct malloc_header *)((uint_32)addr - (uint_32)MALLOC_HEADER);
	if (malloc_point->addr != (uint_32)malloc_point)
	{
		printf("The bad address!\n");
		return;
	}

	/* coalesce with pre */
	malloc_free_pre = (struct malloc_header *)(malloc_point->addr - malloc_point->pre_size);
	if (malloc_point != (struct malloc_header *)malloc_mem && malloc_free_pre->free_list.next != NULL && malloc_free_pre->free_list.pre != NULL)
		coalesce_case |= 1;

	malloc_free_next = (struct malloc_header *)(malloc_point->addr + malloc_point->size);
	if (malloc_free_next->addr != NULL && malloc_free_next->free_list.next != NULL && malloc_free_next->free_list.pre != NULL)
		coalesce_case |= 2;

	switch(coalesce_case)
	{
	case 0:     /* coalesce with nothing */
	case 3:     /* coalesce with both */
	case 1:     /* coalesce with pre */
		if (coalesce_case & 1)
		{
		    malloc_free_pre->size += malloc_point->size;
			malloc_free_next->pre_size = malloc_free_pre->size;
			malloc_point = malloc_free_pre;
			DList_delete_node(&malloc_free_list, &malloc_free_pre->free_list);
		}
	case 2:     /* coalesce with next */
		if (coalesce_case & 2)
		{
			malloc_point->size += malloc_free_next->size;
			DList_delete_node(&malloc_free_list, &malloc_free_next->free_list);
			malloc_free_next = (struct malloc_header *)(malloc_free_next->addr + malloc_free_next->size);
			malloc_free_next->pre_size = malloc_point->size;
		}
		DList_add_tail(&malloc_free_list, &malloc_point->free_list);
		break;
	default:
		break;
	}

	return;
}
