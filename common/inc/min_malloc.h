#ifndef MIN_MALLOC_H
#define MIN_MALLOC_H

int malloc_debug();
void * min_malloc(int size);
void min_free(void *);

#endif