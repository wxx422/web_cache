#include "../inc/query_record.h"
#include "../inc/query_hashtable.h"
#include <stdio.h>
#include <string.h>

#include <openssl/md5.h>

int init_space_record(void *s, unsigned long size, space_t *space, unsigned int arg)
{
	record_t *p = (record_t *)(s+sizeof(space_t));
	space->first_record = p;
	char *q = (char *)((char *)p+sizeof(record_t));	//注意这里的p，计算的时候将p弱化成char
	while(q<=(char *)(space->hash))
	{
		memset(p, 0, sizeof(record_t));
		//下面两个指针的初始化已经被memset做掉了
		//INIT_LIST_HEAD(&(p->list_record));
		//INIT_HLIST_NODE(&(p->hash_node));

		list_add(&(p->list_record), &(space->head));
		p = (record_t *)q;
		q = (char *)((char *)p+sizeof(record_t));

		space->stat.records_num++;
	}
	space->stat.record_free = space->stat.records_num;


	printf("records_num: %d\n", space->stat.records_num);
	printf("record_free: %d\n", space->stat.record_free);

	return 0;
}

record_t *get_free_record(space_t *space)
{
	record_t *record;
	record = ( (space->head.next==&(space->tail))?NULL:(record_t *)(list_entry(space->head.next, record_t, list_record)));
	if(record != NULL)
	{
		space->stat.record_free--;
	}
	else
	{
		//处理原则待定，先预留
		fprintf(stderr, "结点不够，该记录不予存储！\n");
	}
	
	//设置标志位，移动到使用结点部分
	set_record_flags(record, RECORD_USE);
	list_move(&(record->list_record), &(space->tail));
	return record;
}

int free_record(record_t *record, space_t *space)
{
	//循环链表位置的移动，hash表位置的移动
	unmount_record_from_hash(record);
	__list_del_entry(&(record->list_record));

	memset(record, 0, sizeof(record_t));

	list_add(&(record->list_record), &(space->head));

	space->stat.record_free++;
	return 0;
}

//int url_out_to_in(const char *url_out, char *url_in)
int md5_url(const char *url_out, int len, unsigned char *url_md5_in)
{
	//留住文件名部分即可
	url_md5_in = MD5(url_out, len, url_md5_in);
	return 0;
}

int empty_records(space_t *space)
{
	struct list_head *p;
	record_t *record;

	for(p=&(space->tail.next); p!=&(space->hot_boundary); p=p->next)
	{
		record = list_entry(p, record_t, list_record);
		free_record(record, space);
	}

	for(p=&(space->hot_boundary.next); p!=&(space->head); p=p->next)
	{
		record = list_entry(p, record_t, list_record);
		free_record(record, space);
	}

	return 0;
}
