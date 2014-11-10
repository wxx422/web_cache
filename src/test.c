#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../inc/query_types.h"
#include "../inc/query_space.h"
#include "../inc/process_url.h"

#include <sys/stat.h>
//#include <time.h>
#include <sys/time.h>

//#define DEBUG_WEB_CACHE 1

#define TEST_FILE "/root/Desktop/data_big.txt"
#define TEST_RECORD_NUM 1135880


typedef struct _test_t
{
	char name[100];
	struct list_head list;
}test_t;


typedef struct _testb_t
{
	char name[100];
	struct hlist_node hash_node;
}testb_t;

int list_test()
{
	test_t a, b, c;

	strcpy(a.name, "aaa\0");
	strcpy(b.name, "bbb\0");
	strcpy(c.name, "ccc\0");

	INIT_LIST_HEAD(&(a.list));

	list_add(&(b.list), &(a.list));
	list_add(&(c.list), &(b.list));



	test_t *p = &a;
	printf("%s\n", p->name);
	printf("%02x\n", p);
	list_for_each_entry(p, &(a.list), list)
	{
		printf("%s\n", p->name);
		printf("%02x\n", p);
	}

	return 0;
}

int hash_test()
{
	testb_t b, c, d;
	memset(&b, 0, sizeof(b));
	memset(&c, 0, sizeof(c));
	memset(&d, 0, sizeof(d));

	strcpy(b.name, "bbb\0");
	strcpy(c.name, "ccc\0");
	strcpy(d.name, "ddd\0");


	struct hlist_head a;
	INIT_HLIST_HEAD(&a);

	hlist_add_head(&(b.hash_node), &a);
	hlist_add_head(&(c.hash_node), &a);
	hlist_add_head(&(d.hash_node), &a);

	testb_t *p;
	hlist_for_each_entry(p, &a, hash_node)
	{
		printf("%s\n", p->name);
	}

	return 0;
}


int main()
{
	printf("strat list test\n");
	list_test();
	printf("\n*************\n");
	printf("strat hash test\n");
	hash_test();

	void *s=NULL;
	init_query_url(&s);


	if(s!=NULL)
	{
		printf("申请完毕！\n");
		printf("开始处理数据\n");
		struct timeval start_time, end_time;

		//start_time = clock();
		gettimeofday(&start_time, NULL);
		insert_records_from_file(TEST_FILE, (space_t *)s);
		//end_time = clock();
		gettimeofday(&end_time, NULL);
		printf("处理数据结束\n");
		printf("经过%d微秒\n", 1000000*(end_time.tv_sec - start_time.tv_sec)+(end_time.tv_usec - start_time.tv_usec));

		if(dest_space(s)!=0)
		{
			printf("释放空间出错！\n");
		}
		else
		{
			printf("释放完毕！\n");
		}
	}

	return 0;
}

//插入16W条数据，进行测试
unsigned int get_file_length(const char *filename)
{
	unsigned long filesize = 0;
	struct stat statbuf;
	if(stat(filename, &statbuf)<0)
	{
		return filesize;
	}
	else
	{
		filesize=statbuf.st_size;
	}
	return filesize;
}

int insert_records_from_file(const char *filename, space_t *space)
{
	unsigned long filesize = get_file_length(filename);
	unsigned char *p = (unsigned char *)malloc(filesize+1);
	
	memset(p, 0, filesize);
	
	FILE *fp = fopen(filename, "rb");
	if(fp == NULL)
	{
		fprintf(stderr, "open file error\n");
		free(p);
		return 1;
	}
	
	fread(p, 1, filesize, fp);
	fclose(fp);
	fp=NULL;
	
	unsigned char *q, *r;
	unsigned char temp[3072];
	q=p;
	int i;

	/******************************处理文件，去重***************************************/
	FILE *fp1 = fopen("data_new.txt", "wb");
	if(fp1 == NULL)
	{
		fprintf(stderr, "open file error\n");
		free(p);

		return 1;
	}
	/******************************处理文件，去重***************************************/

	for(i=0;i<TEST_RECORD_NUM;i++)
	{
		r=strstr(q, "\n");
		strncpy(temp, q, (r-q));
		temp[r-q]='\0';

		//处理该url
		//process_url(temp, r-q+1, space);
		process_url_test(temp, r-q+1, space, fp1);

		r+=1;
		q=r;
	}

	free(p);

	/******************************处理文件，去重***************************************/
	fclose(fp1);
	fp1=NULL;
	/******************************处理文件，去重***************************************/

	return 0;
}
