#ifndef _QUERY_TYPES_H
#define _QUERY_TYPES_H 1

#include "list.h"

#ifdef __cplusplus
extern "C"
{
#endif


//宏
//状态宏
#define RECORD_UNUSE 0x00000000
#define RECORD_USE 0x80000000
#define RECORD_NOTDOWNLOADED 0x00000000
#define RECORD_UNMATCH 0x00800000
#define RECORD_DOWNING 0x01000000
#define RECORD_DOWNLOADED 0x01800000
//#define OUT_URL_LENGTH 2048
//#define IN_URL_LENGTH 1024
//#define NAME_LENGTH 512
#define SPACE_SIZE 0x20000000		//即为512M(512*1024*1024)
#define HASH_ITME_COUNT 0x100000	//hash下标范围为0～～0xFFFFF
//实验时给的空间大小为512M，每个哈系链表头为一个指针的空间，8字节，每个链表结点空间对齐之后是80字节，
//若按照每个链表长度是4来计算，共需要约为512*1024*1024/（8+4*80）～～163W的哈系表长

//设置标志（可复合）
#define set_record_flags(record, flags) if(record!=NULL)record->flag|=(flags)
//取消标志（可复合）
#define unset_record_flags(record, flags) if(record!=NULL)record->flag&=(~(flags))
//判断标志（不可复合）
#define is_record_flag(record, cflag)(record==NULL?0:record->flag&cflag)

typedef union
{
	unsigned char url_md5_char[16];
	struct
	{
		unsigned long long high8;
		unsigned long long low8;
	}url_md5_long;
}url_md5;

//记录结构体
typedef struct _record_t
{
	/*
	unsigned long hash;					//hash索引
	char out_URL[OUT_URL_LENGTH];				//外网url
	char in_URL[IN_URL_LENGTH];				//内网url
	char resource_name[NAME_LENGTH];			//资源名
	*/
	/*union
	{
		unsigned char url_md5_char[16];
		struct
		{
			unsigned long long high8;
			unsigned long long low8;
		}url_md5_long;
	}url_md5;*/

	url_md5 url;

	unsigned int url_id;						//数据库中存储的id,内部url可以直接用这个url

	unsigned int hit_times;						//访问次数
	unsigned long first_hit_time;				//首次访问时间
	unsigned long last_hit_time;				//上次访问时间
	/*
	unsigned long resource_size;				//资源大小
	unsigned int download_times;				//被下载次数
	unsigned long download_size;				//服务流量
	*/
	unsigned int flag;							/*32位标志，从高位开始使用
												1位：0不使用，1使用
												23位：00未下载，01不符合要求不下载，10正在下载，11已经下载
	 	 	 	 	 	 	 	 	 	 	 	 */
	struct list_head list_record;				//记录的循环链表结构
	struct hlist_node hash_node;				//哈系开链节点
}record_t;

//统计信息结构体
typedef struct _stat_t
{
	unsigned long records_num;					//记录总数
	unsigned long record_free;					//空闲记录个数
	unsigned long hot_records_num;				//热点记录数
	unsigned long seek_times;					//查找总数
	unsigned long seek_success_times;			//查找成功总数
	unsigned long download_times;				//下载总数
	unsigned long download_success_times;		//下载成功总数
	unsigned long unmatch_tactics_times;		//不符合策略总数
}stat_t;

//空间摘要信息
typedef struct _space_t
{
	char *space_head;							//空间首地址
	record_t *first_record;						//空间上第一个记录结构体地址
	struct list_head head;						//头结点
	struct list_head tail;						//尾结点
	struct list_head hot_boundary;				//分界结点
												/*
												 * 头结点和尾结点之间是空闲结点，尾结点和分界结点之间是非热点结点
												 * 分解结点和头结点之间是热点结点
												 * */

	//record_t *hot_boundary;						//记录链表中第一个不是hot资源的记录

	struct hlist_head *hash;					//哈希表首地址
	unsigned long hash_itme_count;				//哈系表大小
	stat_t stat;								//统计信息
}space_t;

#ifdef __cplusplus
extern "C"
}
#endif

#endif
