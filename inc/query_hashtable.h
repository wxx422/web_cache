#ifndef __QUERY_HASHTABLE_H_
#define __QUERY_HASHTABLE_H_ 1

#include "query_types.h"

int init_space_hash(void *s, unsigned long size, space_t *space, unsigned int arg);

unsigned long hash_fg(unsigned char *out_url);

int mount_record_to_hash(record_t *record, unsigned long hash_index, space_t *space);

int unmount_record_from_hash(record_t *record);

record_t *find_record_from_hash(unsigned long hash_index, const url_md5 *url, space_t *space);

#endif
