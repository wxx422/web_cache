#ifndef __QUERY_SPACE_H_
#define __QUERY_SPACE_H_ 1

#include "query_types.h"


int init_query_url(void **s);
//int init_query_url(void *s);

int init_space(void *s, unsigned long size, space_t *space, unsigned int arg);

int dest_space(void *s);

//int dest_space();

#endif
