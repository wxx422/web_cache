#include "../inc/query_types.h"
#include "../inc/query_space.h"

#include <unistd.h>
#include <errno.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>


#include <pthread.h>

//#define TEST_FILE "/root/Desktop/data_big.txt"
//#define TEST_RECORD_NUM 1135880


#define TEST_FILE "/root/Desktop/down_test.txt"
#define TEST_RECORD_NUM 1005

#define MSG_MAX_SIZE 3072		//消息最大长度
#define MSG_BACK_MAX_SIZE 12	//回传消息最大长度
#define IDENTIFIER_SEND 12345
#define IDENTIFIER_RECV 13579

typedef struct msg_web_cache_t_
{
	long msgtype;
	//消息内容前4个字节是flag，接着是8字节的双重指针，然后是外部url地址
	unsigned char mtext[MSG_MAX_SIZE];
}msg_web_cache_t;

typedef struct msg_bcak_web_cache_t_
{
	long msgtype;
	//消息前4个字节是flag,接着是8字节的双重指针
	unsigned char mtext[MSG_BACK_MAX_SIZE];
}msg_back_web_cache_t;

typedef struct att_thread_t_
{
	int msgid_recv;
	space_t *space;
	pthread_mutex_t node_lock;
}att_thread_t;



int insert_records_from_file(const char *filename, space_t *space, int msgid_send);
int send_msg(char *url, record_t *record, int msgid_send);
void *listen_result(void *arg);
int att_alloc(att_thread_t *p, int msgid, void *space);
unsigned int get_file_length(const char *filename);


int send_msg(char *url, record_t *record, int msgid_send)
{
	if(url!=NULL)
	{
		msg_web_cache_t msg;
		//init msg
		msg.msgtype = 1;
		int *temp1 = (int *)msg.mtext;
		*temp1 = 0;
		void **temp2 = (void **)(&msg.mtext[4]);
		*temp2 = (void *)record;

		unsigned char *url_msg = (unsigned char *)(&msg.mtext[12]);
		strcpy(url_msg, url);
		//send msg
		if(msgsnd(msgid_send, (void *)&msg, MSG_MAX_SIZE, 0)==-1)
		{
			fprintf(stderr, "msgsnd error");
			return 1;
		}
		else
		{
			set_record_flags(record, RECORD_DOWNLOADED);
		}
		return 0;
	}
	return 1;
}

void *listen_result(void *arg)
{
	att_thread_t *p = (att_thread_t *)arg;
	long int msgtype = 0;
	msg_back_web_cache_t data;

	record_t **record;
	int *flag;
	int count = 0;
	while(1)
	{
		printf("seq:%d\n", count++);
		//不断接收消息，并且将记录的标志位置上
		if(msgrcv(p->msgid_recv, (void *)&data, MSG_BACK_MAX_SIZE, msgtype, 0)==-1)
		{
			fprintf(stderr, "msgrcv failed with error:%d\n", errno);
			//exit(EXIT_FAILURE);
		}
		else
		{
			flag = (int *)(data.mtext);
			record = (record_t **)(&data.mtext[4]);
			int i;
			for(i=4; i<12; i++)
			{
				printf("%02x", data.mtext[i]);
			}
			printf("\n");
			if(*record==NULL)
			{
				fprintf(stderr, "pointer of node transfer error\n");
				continue;
			}
			if(is_record_flag((*record), RECORD_DOWNING))
			{
				unset_record_flags((*record), RECORD_DOWNING);
				if(*flag == 3)
				{
					//设置标志位
					set_record_flags((*record), RECORD_DOWNLOADED);
					printf("已设置记录标志为已下载\n");
					//移动节点，因为两个线程都涉及到对结点的移动，那在此就需要同步机制。先采用互斥锁的方式
					pthread_mutex_lock(&p->node_lock);
					list_move(&((*record)->list_record), &(p->space->hot_boundary));
					pthread_mutex_unlock(&p->node_lock);
					printf("已将记录移动至热点区域\n");
				}
				else if(*flag == 1)
				{
					set_record_flags((*record), RECORD_UNMATCH);
				}
			}
		}
		sleep(2);
	}
}

int att_alloc(att_thread_t *p, int msgid_recv, void *space)
{
	//p=(att_thread_t *)malloc(sizeof(att_thread_t));
	if(p!=NULL)
	{
		if(pthread_mutex_init(&p->node_lock, NULL)!=0)
		{
			fprintf(stderr, "pthread_mutex_init failed\n");
			free(p);
			return NULL;
		}
		p->msgid_recv = msgid_recv;
		p->space = (space_t *)space;
	}
	return 0;
}

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

int insert_records_from_file(const char *filename, space_t *space, int msgid_send)
{
	printf("进到文件操作记录函数\n");
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

	for(i=0;i<TEST_RECORD_NUM;i++)
	{
		r=strstr(q, "\n");
		strncpy(temp, q, (r-q));
		temp[r-q]='\0';

		//处理该url
		process_url(temp, r-q+1, space, msgid_send);

		r+=1;
		q=r;
	}
	free(p);
	return 0;
}


int main()
{
	//主线程：
	//1、初始化内存
	//2、初始化消息队列，用于发送消息。在此消息队列标识符为12345。
	//3、创建新线程。新线程用于接收消息。

	printf("初始化内存区域\n");
	void *s=NULL;
	init_query_url(&s);
	printf("初始化完成\n");

	//msgid to recv and send msg, init in main
	int msgid_send = -1;
	int msgid_recv = -1;

	int err;
	pthread_t pid_listen;
	att_thread_t att;

	if(s!=NULL)
	{
		printf("初始化消息队列---发送下载任务\n");
		msgid_send = msgget((key_t)IDENTIFIER_SEND, 0666|IPC_CREAT);
		if(msgid_send == -1)
		{
			fprintf(stderr, "msgget failed with error:%d\n", errno);
			exit(EXIT_FAILURE);
		}
		printf("初始化消息队列完成\n");

		printf("初始化消息队列---接受回复信息\n");
		msgid_recv = msgget((key_t)IDENTIFIER_RECV, 0666|IPC_CREAT);
		if(msgid_recv == -1)
		{
			fprintf(stderr, "msgget failed with error:%d\n", errno);
			exit(EXIT_FAILURE);
		}
		printf("初始化消息队列完成\n");

		printf("创建新线程---接受回复信息线程\n");
		att_alloc(&att, msgid_recv, s);
		err = pthread_create(&pid_listen, NULL, listen_result, (void *)&att);
		printf("创建新线程完成\n");
	}

	printf("开始处理文件中的记录\n");
	/*模拟文件处理过程*/
	insert_records_from_file(TEST_FILE, (space_t *)s, msgid_send);
	printf("记录处理完毕\n");

	void *res;
	err = pthread_join(pid_listen, &res);

	return 0;
}
