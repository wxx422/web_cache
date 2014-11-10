#include "../inc/query_space.h"
#include "../inc/query_hashtable.h"
#include "../inc/query_record.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
int init_query_url(void *s)
{
	unsigned long size = SPACE_SIZE;
	if(s==NULL)
	{
		fprintf(stderr, "malloc error!\n");
		return 1;
	}
	if(init_space(s, size, (space_t *)s, 0)!=0)
	{
		fprintf(stderr, "init_space error. File: %s\n", __FILE__);
		free(s);
		s=NULL;
		return 1;
	}
	else
	{
		return 0;
	}
}
*/

int init_query_url(void **s)
{
	unsigned long size = SPACE_SIZE;
	*s = malloc(size);
	if(*s==NULL)
	{
		fprintf(stderr, "malloc error!\n");
		return 1;
	}
	if(init_space(*s, size, (space_t *)*s, 0)!=0)
	{
		fprintf(stderr, "init_space error. File: %s\n", __FILE__);
		free(*s);
		*s=NULL;
		return 1;
	}
	else
	{
		return 0;
	}
}


int init_space(void *s, unsigned long size, space_t *space, unsigned int arg)
{
	memset(&(space->stat), 0, sizeof(stat_t));
	space->space_head = (char *)s;

	printf("space_head地址为%02x\n", space->space_head);

	INIT_LIST_HEAD(&(space->head));
	list_add(&(space->tail), &(space->head));
	list_add(&(space->hot_boundary), &(space->tail));

	init_space_hash(s, size, space, arg);
	init_space_record(s, size, space, arg);

	return 0;
}

int dest_space(void *s)
{
	if(s!=NULL)
	{
		free(s);
		s = NULL;
		return 0;
	}
	return 1;
}

