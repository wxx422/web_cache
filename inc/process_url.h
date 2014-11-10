#ifndef __PROCESS_URL_H_
#define __PROCESS_URL_H_ 1

#include "query_types.h"

int process_url(const unsigned char *temp, unsigned int len, space_t *space, int msgid_send);

#endif
