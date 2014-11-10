#include "../inc/query_types.h"
#include "../inc/query_space.h"

#include <unistd.h>
#include <errno.h>
#include <sys/msg.h>

#include <pthread.h>

#define MAX_SIZE 2048
#define IDENTIFIER 12345

typedef struct msg_web_cache_t_
{
	long msgtype;
	char url[MAX_SIZE];
	int flag;
	void *node; //用于传递结点指针
}msg_web_cache_t;

typedef struct att_thread_t_
{
	int msgid;
	space_t *space;
	pthread_mutex_t node_lock;
}att_thread_t;

void *listen_result(void *arg)
{
	att_thread_t *p = (att_thread_t *)arg;
	long int msgtype = 0;
	msg_web_cache_t data;

	record_t *record;

	while(1)
	{
		//不断接收消息，并且将记录的标志位置上
		if(msgrcv(p->msgid, (void *)&data, MAX_SIZE, msgtype, 0)==-1)
		{
			fprintf(stderr, "msgrcv failed with error:%d\n", errno);
			exit(EXIT_FAILURE);
		}

		record = (record_t *)data.node;
		if(record==NULL)
		{
			fprintf(stderr, "pointer of node transfer error\n");
			continue;
		}
		if(is_record_flag(record, RECORD_DOWNING))
		{
			unset_record_flags(record, RECORD_DOWNING);
			if(data.flag == 3)
			{
				//设置标志位
				set_record_flags(record, RECORD_DOWNLOADED);
				//移动节点，因为两个线程都涉及到对结点的移动，那在此就需要同步机制。先采用互斥锁的方式
				pthread_mutex_lock(&p->node_lock);
				list_move(&(record->list_record), &(p->space->hot_boundary));
				pthread_mutex_unlock(&p->node_lock);

			}
			else if(data.flag == 1)
			{
				set_record_flags(record, RECORD_UNMATCH);
			}
		}
	}
}

att_thread_t *att_alloc(int msgid, void *space)
{
	att_thread_t *p;
	p=(att_thread_t *)malloc(sizeof(att_thread_t));
	if(p!=NULL)
	{
		if(pthread_mutex_init(&p->node_lock, NULL)!=0)
		{
			fprintf(stderr, "pthread_mutex_init failed\n");
			free(p);
			return NULL;
		}
		p->msgid = msgid;
		p->space = (space_t *)space;
	}
	return p;
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
	
	int msgid = -1;

	int err;
	pthread_t pid_listen;
	att_thread_t *att;
	
	if(s!=NULL)
	{
		printf("初始化消息队列\n");
		msgid = msgget((key_t)IDENTIFIER, 0666|IPC_CREAT);
		if(msgid == -1)
		{
			fprintf(stderr, "msgget failed with error:%d\n", errno);
			exit(EXIT_FAILURE);
		}
		printf("初始化消息队列完成\n");

		printf("创建新线程\n");
		att = att_alloc(msgid, s);
		err = pthread_create(&pid_listen, NULL, listen_result, (void *)&att);
		printf("创建新线程完成\n");
	}
	
	return 0;
}
