#pragma once
#include "RingBuffer.h"


struct Header
{
	unsigned char byCode;			// ��Ŷ�ڵ� 0x89 ����.
	unsigned char bySize;			// ��Ŷ ������.
	unsigned char byType;			// ��ŶŸ��.
};
#define dfPACKET_MOVE_DIR_LL					0
#define dfPACKET_MOVE_DIR_LU					1
#define dfPACKET_MOVE_DIR_UU					2
#define dfPACKET_MOVE_DIR_RU					3
#define dfPACKET_MOVE_DIR_RR					4
#define dfPACKET_MOVE_DIR_RD					5
#define dfPACKET_MOVE_DIR_DD					6
#define dfPACKET_MOVE_DIR_LD					7
#define dfPACKET_MOVE_DIR_NOMOVE				8



struct Session
{
	int id;
	SOCKET clientSock;
	RingBuffer recvBuffer;
	RingBuffer sendBuffer;
	Session(SOCKET sock, int ID);
};

bool NetworkInitAndListen();
bool NetworkProc();
void disconnect(int id);
void deleteDisconnected();
void ClearSessionInfo();
void Update();


