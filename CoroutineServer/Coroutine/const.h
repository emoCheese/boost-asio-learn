#pragma once


#define MAX_LENGTH 1024 * 2 // �������ݳ���
#define MAX_SENDQUE 1000

#define HEAD_ID_LEN 2       // ͷ��id���ֽڳ���
#define HEAD_DATA_LEN 2     // ͷ�����ݳ��ȵ��ֽڳ���
#define HEAD_TOTAL_LEN 4    // �ܵ�ͷ�����ֽڳ���

enum Msg_IDS
{
	MSG_HELLO_WORLD = 1001
};