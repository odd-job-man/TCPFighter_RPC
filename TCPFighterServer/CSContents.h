#pragma once
#include "Network.h"
#include "CStub.h"
constexpr int ERROR_RANGE = 50;
constexpr int MOVE_UNIT_X = 3;
constexpr int MOVE_UNIT_Y = 2;

constexpr char INIT_DIR = dfPACKET_MOVE_DIR_LL;
constexpr int INIT_POS_X = 300;
constexpr int INIT_POS_Y = 300;
constexpr int INIT_HP = 100;
constexpr int dfRANGE_MOVE_TOP = 50;
constexpr int dfRANGE_MOVE_LEFT = 10;
constexpr int dfRANGE_MOVE_RIGHT = 630;
constexpr int dfRANGE_MOVE_BOTTOM = 470;

//---------------------------------------------------------------
// 공격범위.
//---------------------------------------------------------------
constexpr int dfATTACK1_RANGE_X = 80;
constexpr int dfATTACK2_RANGE_X = 90;
constexpr int dfATTACK3_RANGE_X = 100;
constexpr int dfATTACK1_RANGE_Y = 10;
constexpr int dfATTACK2_RANGE_Y = 10;
constexpr int dfATTACK3_RANGE_Y = 20;

struct ClientInfo
{
	int ID;
	int hp;
	int x;
	int y;
	char viewDir;
	int action;
};

inline bool IsCollision_ATTACK1(IN ClientInfo* pAttackerCI, IN ClientInfo* pAttackedCI)
{
	int dX = abs(pAttackerCI->x - pAttackedCI->x);
	int dY = abs(pAttackerCI->y - pAttackedCI->y);
	bool isValidX = (dX <= dfATTACK1_RANGE_X) ? true : false;
	bool isValidY = (dY <= dfATTACK1_RANGE_Y) ? true : false;
	return isValidX && isValidY;
}

inline bool IsCollision_ATTACK2(IN ClientInfo* pAttackerCI, IN ClientInfo* pAttackedCI)
{
	int dX = abs(pAttackerCI->x - pAttackedCI->x);
	int dY = abs(pAttackerCI->y - pAttackedCI->y);
	bool isValidX = (dX <= dfATTACK2_RANGE_X) ? true : false;
	bool isValidY = (dY <= dfATTACK2_RANGE_Y) ? true : false;
	return isValidX && isValidY;
}

inline bool IsCollision_ATTACK3(IN ClientInfo* pAttackerCI, IN ClientInfo* pAttackedCI)
{
	int dX = abs(pAttackerCI->x - pAttackedCI->x);
	int dY = abs(pAttackerCI->y - pAttackedCI->y);
	bool isValidX = (dX <= dfATTACK3_RANGE_X) ? true : false;
	bool isValidY = (dY <= dfATTACK3_RANGE_Y) ? true : false;
	return isValidX && isValidY;
}

#define IN
#define OPTIONAL
#define OUT
void Update();



