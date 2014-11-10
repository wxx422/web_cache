#ifndef QUERY_RECORD_H_
#define QUERY_RECORD_H_ 1

#include "query_types.h"

int init_space_record(void *s, unsigned long size, space_t *space, unsigned int arg);

record_t *get_free_record(space_t *space);

int free_record(record_t *record, space_t *space);

//int url_out_to_in(const char *url_out, char *url_in);
int md5_url(const char *url_out, int len, unsigned char *url_md5_in);

int empty_records();

#endif
