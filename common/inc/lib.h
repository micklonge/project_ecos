#ifndef LIB_H
#define LIB_H

void * com_malloc_log(char *, char *, int , int);
void * com_realloc_log(char *, char *, int, void * buffer, int size);
void * com_free_log(char *, char *, int , void *);

#define com_malloc(x) com_malloc_log(__FILE__, __FUNCTION__, __LINE__, x)
#define com_realloc(x, y) com_realloc_log(__FILE__, __FUNCTION__, __LINE__, x, y)
#define com_free(x) com_free_log(__FILE__, __FUNCTION__, __LINE__, x)

#endif