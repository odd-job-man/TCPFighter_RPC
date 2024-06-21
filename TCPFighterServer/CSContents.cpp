#include <WS2spi.h>
#include <WS2tcpip.h>
#include <windows.h>
#include "SerializeBuffer.h"
#include "SCContents.h"
#include "CStub.h"
#include "CSContents.h"
#include <map>
#include <set>
#include "Network.h"
#define IN
#define OUT
#define LOG

extern int g_IdToAlloc;
std::map<int, ClientInfo*> g_clientMap;
extern SerializeBuffer g_sb;
#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
CSProc g_CSProc;
extern std::set<int> g_disconnected_id_set;


void InitClientInfo(OUT ClientInfo* pCI, IN int id)
{
	pCI->ID = id;
	pCI->hp = INIT_HP;
	pCI->x = INIT_POS_X;
	pCI->y = INIT_POS_Y;
	pCI->action = dfPACKET_MOVE_DIR_NOMOVE;
	pCI->viewDir = INIT_DIR;
#ifdef LOG
	wprintf(L"Create Character # SessionID : %d\tX:%d\tY:%d\n", id, INIT_POS_X, INIT_POS_Y);
#endif
}

void ClearClientInfo()
{
	for (auto iter = g_clientMap.begin(); iter != g_clientMap.end();)
	{
		delete iter->second;
		iter = g_clientMap.erase(iter);
	}
	g_clientMap.clear();
}
inline bool isDeletedSession(IN const int id)
{
	return g_disconnected_id_set.find(id) != g_disconnected_id_set.end();
}
inline void HandleMoving(IN ClientInfo* pCI, IN int moveDir)
{
	bool isNoX = false;
	bool isNoY = false;

	if (pCI->x - MOVE_UNIT_X <= dfRANGE_MOVE_LEFT || pCI->x + MOVE_UNIT_X >= dfRANGE_MOVE_RIGHT)
	{
		isNoX = true;
	}

	if (pCI->y - MOVE_UNIT_Y <= dfRANGE_MOVE_TOP || pCI->y + MOVE_UNIT_Y >= dfRANGE_MOVE_BOTTOM)
	{
		isNoY = true;
	}

	switch (moveDir)
	{
	case dfPACKET_MOVE_DIR_LL:
		if (!isNoX)
		{
			pCI->x -= MOVE_UNIT_X;
#ifdef LOG
			wprintf(L"Session ID : %d #\tDIR : LL\tMove Simulate X : %d, Y : %d\n", pCI->ID, pCI->x, pCI->y);
#endif
		}
		break;
	case dfPACKET_MOVE_DIR_LU:
		if (!isNoX && !isNoY)
		{
			pCI->x -= MOVE_UNIT_X;
			pCI->y -= MOVE_UNIT_Y;
#ifdef LOG
			wprintf(L"Session ID : %d #\tDIR : LU\tMove Simulate X : %d, Y : %d\n", pCI->ID, pCI->x, pCI->y);
#endif
		}
		break;
	case dfPACKET_MOVE_DIR_UU:
		if (!isNoY)
		{
			pCI->y -= MOVE_UNIT_Y;
#ifdef LOG
			wprintf(L"Session ID : %d #\tDIR : UU\tMove Simulate X : %d, Y : %d\n", pCI->ID, pCI->x, pCI->y);
#endif
		}
		break;
	case dfPACKET_MOVE_DIR_RU:
		if (!isNoX && !isNoY)
		{
			pCI->x += MOVE_UNIT_X;
			pCI->y -= MOVE_UNIT_Y;
#ifdef LOG
			wprintf(L"Session ID : %d #\tDIR : RU\tMove Simulate X : %d, Y : %d\n", pCI->ID, pCI->x, pCI->y);
#endif
			break;
		}
	case dfPACKET_MOVE_DIR_RR:
		if (!isNoX)
		{
			pCI->x += MOVE_UNIT_X;
#ifdef LOG
			wprintf(L"Session ID : %d #\tDIR : RR\tMove Simulate X : %d, Y : %d\n", pCI->ID, pCI->x, pCI->y);
#endif
		}
		break;
	case dfPACKET_MOVE_DIR_RD:
		if (!isNoX && !isNoY)
		{
			pCI->x += MOVE_UNIT_X;
			pCI->y += MOVE_UNIT_Y;
#ifdef LOG
			wprintf(L"Session ID : %d #\tDIR : RD\tMove Simulate X : %d, Y : %d\n", pCI->ID, pCI->x, pCI->y);
#endif
		}
		break;
	case dfPACKET_MOVE_DIR_DD:
		if (!isNoY)
		{
			pCI->y += MOVE_UNIT_Y;
#ifdef LOG
			wprintf(L"Session ID : %d #\tDIR : DD\tMove Simulate X : %d, Y : %d\n", pCI->ID, pCI->x, pCI->y);
#endif
		}
		break;
	case dfPACKET_MOVE_DIR_LD:
		if (!isNoX && !isNoY)
		{
			pCI->x -= MOVE_UNIT_X;
			pCI->y += MOVE_UNIT_Y;
#ifdef LOG
			wprintf(L"Session ID : %d #\tDIR : LD\tMove Simulate X : %d, Y : %d\n", pCI->ID, pCI->x, pCI->y);
#endif
		}
		break;
	default:
		__debugbreak();
		return;
	}
}
void __forceinline DamageNotify(ClientInfo* pAttackerCI, ClientInfo* pVictimCI)
{
	for (auto iter = g_clientMap.begin(); iter != g_clientMap.end(); ++iter)
	{
		ClientInfo* pOtherCI = iter->second;
		//if (pOtherCI != pAttackerCI)
		//{
			SC_DAMAGE(pOtherCI->ID, pAttackerCI->ID, pVictimCI->ID, pVictimCI->hp);
		//}
	}
}


bool CSProc::CS_MOVE_START(int fromId, char moveDir, unsigned short x, unsigned short y)
{
	ClientInfo* pStartCI = g_clientMap.find(fromId)->second;
	bool isNotValidPos = abs(pStartCI->x - x) > ERROR_RANGE || abs(pStartCI->y - y) > ERROR_RANGE;
	if (isNotValidPos)
	{
#ifdef LOG
		wprintf(L"Session ID : %d\t PROCESS_PACKET_CS_MOVE_START#\tMOVE_RANGE_ERROR - disconnected\n", fromId);
		wprintf(L"Server X : %d, Y : %d, Client X : %d, Y : %d\n", pStartCI->x, pStartCI->y, x, y);
#endif
		disconnect(fromId);
		return false;
	}
#ifdef LOG
	wprintf(L"Session ID : %d\tPACKET_CS_MOVE_START PROCESSING #\tMoveDir : %d\tX:%d\tY:%d\n", fromId, moveDir, x, y);
#endif
	switch (moveDir)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		pStartCI->viewDir = dfPACKET_MOVE_DIR_RR;
		break;
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
		pStartCI->viewDir = dfPACKET_MOVE_DIR_LL;
		break;
	default:
		//__debugbreak();
		break;
	}
	pStartCI->x = x;
	pStartCI->y = y;
	pStartCI->action = moveDir;

	for (auto iter = g_clientMap.begin(); iter != g_clientMap.end(); ++iter)
	{
		ClientInfo* pOtherCI = iter->second;
		if (pOtherCI != pStartCI)
		{
			SC_MOVE_START(pOtherCI->ID, fromId, moveDir, x, y);
		}
	}
	return true;
}

bool CSProc::CS_MOVE_STOP(int fromId, char viewDir, unsigned short x, unsigned short y)
{
	ClientInfo* pStopCI = g_clientMap.find(fromId)->second;
	bool isNotValidPos = abs(pStopCI->x - x) > ERROR_RANGE || abs(pStopCI->y - y) > ERROR_RANGE;
	if (isNotValidPos)
	{
#ifdef LOG
		wprintf(L"Session ID : %d\t PROCESS_PACKET_CS_MOVE_STOP#\tMOVE_RANGE_ERROR - disconnected\n", fromId);
		wprintf(L"Server X : %d, Y : %d, Client X : %d, Y : %d\n", pStopCI->x, pStopCI->y, x, y);
#endif
		disconnect(fromId);
		return false;
	}
#ifdef LOG
	wprintf(L"Session ID : %d\tPACKET_CS_MOVE_STOP PROCESSING #\tMoveDir : %d\tX:%d\tY:%d\n", fromId, viewDir, x, y);
#endif
	pStopCI->viewDir = viewDir;
	pStopCI->x = x;
	pStopCI->y = y;
	pStopCI->action = dfPACKET_MOVE_DIR_NOMOVE;
	for (auto iter = g_clientMap.begin(); iter != g_clientMap.end(); ++iter)
	{
		ClientInfo* pOtherCI = iter->second;
		if (pOtherCI->ID == fromId)
		{
			continue;
		}
#ifdef LOG
		wprintf(L"Session ID : %d -> Other\tPAKCET_SC_MOVE_STOP BroadCast\n", pOtherCI->ID);
#endif
		SC_MOVE_STOP(pOtherCI->ID, fromId, viewDir, x, y);
	}
	return true;
}

bool CSProc::CS_ATTACK1(int fromId, char viewDir, unsigned short x, unsigned short y)
{
#ifdef LOG
	wprintf(L"Session ID : %d\t PROCESS_PACKET_CS_ATTACK1#\n", fromId);
#endif

	ClientInfo* pAttackerCI = g_clientMap.find(fromId)->second;
	if (!(pAttackerCI->x == x && pAttackerCI->y == y))
		__debugbreak();

#ifdef LOG
	wprintf(L"Session ID : %d -> Other \t PROCESS_PACKET_SC_ATTACK1 BroadCast#\n", fromId);
#endif
	for (auto iter = g_clientMap.begin(); iter != g_clientMap.end(); ++iter)
	{
		ClientInfo* pOtherCI = iter->second;
		if (pOtherCI->ID != pAttackerCI->ID)
		{
			SC_ATTACK1(pOtherCI->ID, pAttackerCI->ID, pAttackerCI->viewDir, pAttackerCI->x, pAttackerCI->y);
		}
	}

	constexpr int DAMAGE = 2;
	for (auto iter = g_clientMap.begin(); iter != g_clientMap.end(); ++iter)
	{
		ClientInfo* pOtherCI = iter->second;
		if (pOtherCI->ID != pAttackerCI->ID && IsCollision_ATTACK1(pAttackerCI, pOtherCI))
		{
#ifdef LOG
			wprintf(L"Session ID : %d -> Attacked %d\t PROCESS_PACKET_SC_DAMAGE#\n", fromId, pOtherCI->ID);
#endif
			pOtherCI->hp -= DAMAGE;
			DamageNotify(pAttackerCI, pOtherCI);
		}
	}
	return true;
}

bool CSProc::CS_ATTACK2(int fromId, char viewDir, unsigned short x, unsigned short y)
{
#ifdef LOG
	wprintf(L"Session ID : %d\t PROCESS_PACKET_CS_ATTACK2#\n", fromId);
#endif
	ClientInfo* pAttackerCI = g_clientMap.find(fromId)->second;
	if (!(pAttackerCI->x == x && pAttackerCI->y == y))
		__debugbreak();

#ifdef LOG
	wprintf(L"Session ID : %d -> Other \t PROCESS_PACKET_SC_ATTACK2 BroadCast#\n", fromId);
#endif
	for (auto iter = g_clientMap.begin(); iter != g_clientMap.end(); ++iter)
	{
		ClientInfo* pOtherCI = iter->second;
		if (pOtherCI->ID != pAttackerCI->ID)
		{
			SC_ATTACK2(pOtherCI->ID, pAttackerCI->ID, pAttackerCI->viewDir, pAttackerCI->x, pAttackerCI->y);
		}
	}

	constexpr int DAMAGE = 2;
	for (auto iter = g_clientMap.begin(); iter != g_clientMap.end(); ++iter)
	{
		ClientInfo* pOtherCI = iter->second;
		if (pOtherCI->ID != pAttackerCI->ID && IsCollision_ATTACK2(pAttackerCI, pOtherCI))
		{
#ifdef LOG
			wprintf(L"Session ID : %d -> Attacked %d\t PROCESS_PACKET_SC_DAMAGE#\n", fromId, pOtherCI->ID);
#endif
			pOtherCI->hp -= DAMAGE;
			DamageNotify(pAttackerCI, pOtherCI);
		}
	}
	return true;
}

bool CSProc::CS_ATTACK3(int fromId, char viewDir, unsigned short x, unsigned short y)
{
#ifdef LOG
	wprintf(L"Session ID : %d\t PROCESS_PACKET_CS_ATTACK3#\n", fromId);
#endif

	ClientInfo* pAttackerCI = g_clientMap.find(fromId)->second;
	if (!(pAttackerCI->x == x && pAttackerCI->y == y))
		__debugbreak();

#ifdef LOG
	wprintf(L"Session ID : %d -> Other \t PROCESS_PACKET_SC_ATTACK3 BroadCast#\n", fromId);
#endif
	for (auto iter = g_clientMap.begin(); iter != g_clientMap.end(); ++iter)
	{
		ClientInfo* pOtherCI = iter->second;
		if (pOtherCI->ID != pAttackerCI->ID)
		{
			SC_ATTACK3(pOtherCI->ID, pAttackerCI->ID, pAttackerCI->viewDir, pAttackerCI->x, pAttackerCI->y);
		}
	}

	constexpr int DAMAGE = 2;
	for (auto iter = g_clientMap.begin(); iter != g_clientMap.end(); ++iter)
	{
		ClientInfo* pOtherCI = iter->second;
		if (pOtherCI->ID != pAttackerCI->ID && IsCollision_ATTACK3(pAttackerCI, pOtherCI))
		{
#ifdef LOG
			wprintf(L"Session ID : %d -> Attacked %d\t PROCESS_PACKET_SC_DAMAGE#\n", fromId, pOtherCI->ID);
#endif
			pOtherCI->hp -= DAMAGE;
			DamageNotify(pAttackerCI, pOtherCI);
		}
	}
	return true;
}

void Update()
{
	for (auto iter = g_clientMap.begin(); iter != g_clientMap.end(); ++iter)
	{
		ClientInfo* pCI = iter->second;
		if (isDeletedSession(pCI->ID))
		{
			continue;
		}

		if (pCI->action != dfPACKET_MOVE_DIR_NOMOVE)
		{
			HandleMoving(pCI, pCI->action);
		}

		if (pCI->hp <= 0)
		{
#ifdef LOG
			wprintf(L"Session ID : %d Die\t HP : %d\n", pCI->ID, pCI->hp);
#endif
			disconnect(pCI->ID);
		}
	}
	deleteDisconnected();
}

// 클라이언트의 접속을 끊는 함수들
void disconnect(IN int id)
{
	g_disconnected_id_set.insert(id);
#ifdef LOG
	wprintf(L"disconnect Session id : %d#\n", id);
#endif 
}

