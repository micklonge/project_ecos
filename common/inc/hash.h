/*
 ***************************************************************
 *
 *         FILE NAME      : hash.h
 *         ORIGINATOR     : ÕıÓ»—Ô(micklongen)
 *         DATE OF ORIGIN : 2009-4-26
 *
 ***************************************************************
 */

#ifndef HASH_H
#define HASH_H

#define MAX_HASH_HEADER_LEN               64

struct hash_struct_info
{
	int size;
	struct DListHeader hash_list[MAX_HASH_HEADER_LEN];

	int (*hash)(void *);
	int (*cmp)(void*, void*);
	int (*free)(void*);
};

int hash_init(struct hash_struct_info *head, int (*hash)(void *), int (*cmp)(void*, void*), int (*free)(void *));
int get_hash_size(struct hash_struct_info *head);
struct DListHeader * get_hash_list(struct hash_struct_info *head);
struct DListHeader * get_list_header(struct hash_struct_info *head, struct DListNode *src);
struct DListNode * Hash_list_search(struct hash_struct_info *head, struct DListNode *src);
int Hash_list_insert(struct hash_struct_info *head, struct DListNode *src);
struct DListNode * Hash_list_delete(struct hash_struct_info *head, struct DListNode *src);
int Hash_free(struct hash_struct_info *head);

#endif