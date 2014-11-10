#include "../inc/query_hashtable.h"
#include <string.h>
#include <stdio.h>

int init_space_hash(void *s, unsigned long size, space_t *space, unsigned int arg)
{
	int i;
	space->hash_itme_count = HASH_ITME_COUNT;
	space->hash = (struct hlist_head *)( s + size - (space->hash_itme_count) * sizeof(struct hlist_head));


	printf("space->hash地址为%02x\n", space->hash);

	for(i=0; i<space->hash_itme_count; i++)
	{
		INIT_HLIST_HEAD(&(space->hash[i]));
	}


	printf("last hash item address: %02x\n", &(space->hash[i-1]));

	return 0;
}

unsigned long hash_fg(unsigned char *out_url)
{
	unsigned long h, g;
	unsigned char *p;
	p = out_url;

	h = 0;
	while(*p)
	{
		h = ( h << 4 ) + *p++;
		if(g=h&0xF000000L)
			h^=g>>24;
		h&=~g;
	}
	return (h&(HASH_ITME_COUNT-1));
}

int mount_record_to_hash(record_t *record, unsigned long hash_index, space_t *space)
{
	hlist_add_head(&(record->hash_node), &(space->hash[hash_index]));
	return 0;
}

int unmount_record_from_hash(record_t *record)
{
	hlist_del_init(&(record->hash_node));
	return 0;
}


/*
 * 这里需要注意字节序的问题
 */
record_t *find_record_from_hash(unsigned long hash_index, const url_md5 *url, space_t *space)
{
	struct hlist_head *open_link_head;
	open_link_head = &(space->hash[hash_index]);

	record_t *record;

	hlist_for_each_entry(record, open_link_head, hash_node)
	{

		if( (record->url.url_md5_long.high8 == url->url_md5_long.high8) &&
			(record->url.url_md5_long.low8 == url->url_md5_long.low8) )
		{
			return record;
		}
	}
	return NULL;
}
