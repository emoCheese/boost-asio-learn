#pragma once


#define MAX_LENGTH 1024 * 2 // 最大的数据长度
#define MAX_SENDQUE 1000

#define HEAD_ID_LEN 2       // 头部id的字节长度
#define HEAD_DATA_LEN 2     // 头部数据长度的字节长度
#define HEAD_TOTAL_LEN 4    // 总的头部的字节长度

enum Msg_IDS
{
	MSG_HELLO_WORLD = 1001
};