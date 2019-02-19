/*
 *******************************************************************
 *
 *         FILE NAME      : hash.cpp
 *         ORIGINATOR     : ÕıÓ»—Ô(micklongen)
 *         DATE OF ORIGIN : 2009-4-26
 *
 *******************************************************************
 */

#include "stdafx.h"
#include <stdio.h>

#include "..\inc\log.h"
#include "..\inc\list.h"
#include "..\inc\hash.h"
#include "..\inc\lib.h"

struct DListHeader * get_list_header(struct hash_struct_info *head, struct DListNode *src)
{
	if (head == NULL)
	{
		printf("The hash_struct_info head is NULL.\n");
		exit(1);
	}
	if (head->hash_list == NULL)
	{
		printf("The hash_list is NULL.\n");
		exit(1);
	}
	if (src == NULL)
	{
		printf("The src is NULL.\n");
		exit(1);
	}

	return &(head->hash_list[head->hash(src) % head->size]);
}

struct DListNode * Hash_list_search(struct hash_struct_info *head, struct DListNode *src)
{
	struct DListNode *plink;
	struct DListHeader *header;

	if (head == NULL)
	{
		printf("The hash_struct_info head is NULL.\n");
		exit(1);
	}
	if (src == NULL)
	{
		printf("The src is NULL\n");
		exit(1);
	}

	header = get_list_header(head, src);
	plink = DList_get_next(header, &(header->head));
	while(plink != NULL)
	{
		if (head->cmp((void *)plink, (void *)src) == 0)
			break;
		plink = DList_get_next(header, plink);
	}

	return plink;
}

int Hash_list_insert(struct hash_struct_info *head, struct DListNode *src)
{
	struct DListHeader *header;

	if (head == NULL)
	{
		printf("The hash_struct_info head is NULL.\n");
		exit(1);
	}
	if (src == NULL)
	{
		printf("The src is NULL\n");
		exit(1);
	}

	header = get_list_header(head, src);
	DList_add_tail(header, src);

	return 0;
}

struct DListNode * Hash_list_delete(struct hash_struct_info *head, struct DListNode *src)
{
	struct DListHeader *header;

	if (head == NULL)
	{
		printf("The hash_struct_info head is NULL.\n");
		exit(1);
	}
	if (src == NULL)
	{
		printf("The src is NULL\n");
		exit(1);
	}

	header = get_list_header(head, src);
	if (header == NULL)
	{
		printf("The list header is NULL\n");
		exit(1);
	}

	return DList_delete_node(header, src);
}

int get_hash_size(struct hash_struct_info *head)
{
	if (head == NULL)
	{
		printf("The hash_struct_info is NULL.\n");
		return 0;
	}

	return head->size;
}

struct DListHeader * get_hash_list(struct hash_struct_info *head)
{
	if (head == NULL)
	{
		printf("The hash_struct_info is NULL.\n");
		return 0;
	}

	return head->hash_list;
}

int hash_init(struct hash_struct_info *head, int (*hash)(void *), int (*cmp)(void*, void*), int (*free)(void *))
{
	int i;

	if (head == NULL)
	{
		printf("The hash_struct_info head is NULL.\n");
		exit(1);
	}

	for (i = 0; i < MAX_HASH_HEADER_LEN; ++i)
		initialize_Dlist_header(&(head->hash_list[i]));
	head->size = MAX_HASH_HEADER_LEN;
	head->hash = hash;
	head->cmp = cmp;
	head->free = free;

	return 1;
}

int Hash_free(struct hash_struct_info *head)
{
	int i;
	struct DListNode *plink;
	struct DListHeader *header;

	if (head == NULL)
	{
		log_print("The hash_struct_info head is NULL.\n");
		exit(1);
	}

	for (i = 0; i < head->size; ++i)
	{
		header = &head->hash_list[i];
		plink = DList_get_head(header);
	    while(plink != NULL)
	    {
		    DList_delete_node(header, plink);
			if (head->free == NULL)
			{
				log_print("head->free function pointer is NULL.\n");
				exit(1);
			}
			head->free(plink);
			plink = DList_get_head(header);
	    }		
	}

	return 0;
}