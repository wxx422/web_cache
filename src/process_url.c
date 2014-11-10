#include "../inc/process_url.h"
#include "../inc/query_hashtable.h"
#include "../inc/query_record.h"
//#include "../inc/query_types.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <openssl/md5.h>

#define HIT_TIMES_TO_DOWNLOAD 1000

int process_url(const unsigned char *temp, unsigned int len, space_t *space, int msgid_send)
{
	//对于每一条url地址，计算hash值，计算md5值，查找该url
	//若没有该url，得到一个空闲的记录结构体，写数据（url外网地址(数据库部分)，md5值，更新hit_times,first_hit_time,last_hit_time）
	//若有该url，更新hit_times,last_hit_times,并判断是否到达下载的临界次数，若到达，通知下载模块进行下载，并更新状态为正在下载状态
	//若下载模块返回通知不符合策略要求，则更新状态为不符合要求不下载状态。下载模块下载完毕之后，通知已经完成下载，更新状态至已完成下载状态
	//将不符合规则不下载的记录放在hot_bounary结点之前

	space->stat.seek_times++;

	int ret = 0;

	unsigned long hash;
	url_md5 url;
	record_t *record;

	hash = hash_fg(temp);

	unsigned char md5_temp[16], *p;
	MD5(temp, len, md5_temp);

	p = md5_temp;

	url.url_md5_long.high8 = *((unsigned long long *)p);
	url.url_md5_long.low8 = *((unsigned long long *)(p+8));

	record = find_record_from_hash(hash, &url, space);
	if(record == NULL)
	{
		record = get_free_record(space);
		if(record == NULL)
		{
			fprintf(stderr, "获取空闲结点出错\n");
			ret = 1;
			goto PROCESS_URL_EXIT;
		}

		record->url =url;
		record->hit_times++;
		record->first_hit_time = time(NULL);
		record->last_hit_time = record->first_hit_time;

		//将结点挂载到hash表上
		mount_record_to_hash(record, hash, space);
	}
	else
	{
		space->stat.seek_success_times++;

		record->hit_times++;
		record->last_hit_time = time(NULL);

		//因为下载次数的临界不应该为1，所以触发条件的判断放在这里，记录的下载状态为未下载
		//这里涉及到策略模块的东西，暂时留着
		//TODO：
		//printf("%d\n", record->hit_times);
		if( (record->hit_times>HIT_TIMES_TO_DOWNLOAD) && (is_record_flag(record, RECORD_DOWNLOADED)==RECORD_NOTDOWNLOADED))
		//if( (record->hit_times>HIT_TIMES_TO_DOWNLOAD) && (record->flag==RECORD_NOTDOWNLOADED) )
		{
			printf("send msg to other process\n");
			send_msg(temp, record, msgid_send);
		}
	}

PROCESS_URL_EXIT:
	return ret;
}

//
//int process_url_test(const unsigned char *temp, unsigned int len, space_t *space, FILE *fp1)
//{
//	space->stat.seek_times++;
//
//	int ret = 0;
//
//	/******************************处理文件，去重***************************************/
//	unsigned long hash;
//	url_md5 url;
//	record_t *record;
//
//	unsigned char md5_temp[16], *p_md5;
//	/******************************处理文件，去重***************************************/
//
//	MD5(temp, len, md5_temp);
//	p_md5 = md5_temp;
//
//	url.url_md5_long.high8 = *((unsigned long long *)p_md5);
//	url.url_md5_long.low8 = *((unsigned long long *)(p_md5+8));
//
//	hash = hash_fg(temp);
//
//	record = find_record_from_hash(hash, &url, space);
//	if(record == NULL)
//	{
//		fwrite(temp, strlen(temp), 1, fp1);
//		fwrite("\n", strlen("\n"), 1, fp1);
//		record = get_free_record(space);
//		if(record == NULL)
//		{
//			fprintf(stderr, "获取空闲结点出错\n");
//			ret = 1;
//			goto PROCESS_URL_TEST_EXIT;
//		}
//
//		record->url.url_md5_long.low8 =url.url_md5_long.low8;
//		record->url.url_md5_long.high8 =url.url_md5_long.high8;
//		record->hit_times++;
//		record->first_hit_time = time(NULL);
//		record->last_hit_time = record->first_hit_time;
//
//		//将结点挂载到hash表上
//		mount_record_to_hash(record, hash, space);
//	}
//	else
//	{
//		space->stat.seek_success_times++;
//
//		record->hit_times++;
//		record->last_hit_time = time(NULL);
//
//		//因为下载次数的临界不应该为1，所以触发条件的判断放在这里，记录的下载状态为未下载
//		//这里涉及到策略模块的东西，暂时留着
//		//TODO：
//		if((record->hit_times >= HIT_TIMES_TO_DOWNLOAD) && (is_record_flag(record, RECORD_DOWNLOADED)==RECORD_NOTDOWNLOADED)
//		{
//			//发送消息以下载，下载资源的url为temp
//
//
//		}
//	}
//	/******************************处理文件，去重***************************************/
//
//PROCESS_URL_TEST_EXIT:
//	return ret;
//}

