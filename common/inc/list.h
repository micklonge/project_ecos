/*
 ***************************************************************
 *
 *         FILE NAME      : list.h
 *         ORIGINATOR     : ÍõîÈÑï(micklongen)
 *         DATE OF ORIGIN : 2009-4-25
 *
 ***************************************************************
 */

#ifndef LIST_H
#define LIST_H

struct DListNode
{
	struct DListNode* pre;
	struct DListNode* next;
};

struct DListHeader
{
	struct DListNode head;
};

#define initialize_Dlist_header(Header) \
(\
	(Header)->head.pre = &(Header)->head, \
	(Header)->head.next = &(Header)->head \
)

#define DList_empty(Header) \
(\
    ((Header)->head.next == &(Header)->head) \
)

#define DList_get_head(Header) \
(\
	((Header)->head.next == &(Header)->head) \
	? \
	(NULL) \
	: \
	((Header)->head.next) \
)

#define DList_add_head(Header, link) \
( \
	(link)->next = (Header)->head.next,  \
	(link)->next->pre = (link), \
	(Header)->head.next = (link), \
	(link)->pre = &(Header)->head, \
	0 \
)

#define DList_add_tail(Header, link) \
(\
	(link)->pre = (Header)->head.pre, \
	(link)->pre->next = (link), \
	(Header)->head.pre = (link), \
	(link)->next = &(Header)->head,   \
    0 \
)

#define DList_delete_node(Header, link) \
(\
	((Header)->head.next == &(Header)->head) \
	? \
	NULL \
	: \
	(link)->pre->next = (link)->next, \
	(link)->next->pre = (link)->pre, \
	(link)->next = NULL, \
	(link)->pre = NULL, \
	(link)\
)

#define DList_get_next(Header, link) \
(\
	(Header)->head.pre == (link) \
	? \
	NULL \
	: \
	(link)->next \
)

#define DList_get_pre(Header, link) \
(\
	(Header)->head.next == (link) \
	? \
	NULL \
	: \
	(link)->pre \
)

#define DList_add_after(Header, in_list, to_add) \
(\
	((in_list)->next == (in_list)) \
	? \
	(NULL) \
	: \
	(	(to_add)->pre = (in_list), \
		(to_add)->next = (in_list)->next, \
		(in_list)->next = (to_add), \
		(to_add)->next->pre = (to_add), \
		(to_add) \
	) \
)

#define DList_add_before(header, in_list, to_add) \
(	((in_list)->pre == (in_list)) \
	? \
	(NULL) \
	: \
	(	(to_add)->next = (in_list), \
		(to_add)->pre = (in_list)->pre, \
		(in_list)->pre = (to_add), \
		(to_add)->pre->next = (to_add), \
		(to_add) \
	) \
)

#endif