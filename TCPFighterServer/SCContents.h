#pragma once
constexpr int dfPACKET_SC_CREATE_MY_CHARACTER = 0;
constexpr int dfPACKET_SC_CREATE_OTHER_CHARACTER = 1;
constexpr int dfPACKET_SC_DELETE_CHARACTER = 2;
constexpr int dfPACKET_SC_MOVE_START = 11;
constexpr int dfPACKET_SC_MOVE_STOP = 13;
constexpr int dfPACKET_SC_ATTACK1 = 21;
constexpr int dfPACKET_SC_ATTACK2 = 23;
constexpr int dfPACKET_SC_ATTACK3 = 25;
constexpr int dfPACKET_SC_DAMAGE = 30;

int MAKE_HEADER(unsigned char packetSize, unsigned char packetType);
bool SC_CREATE_MY_CHARACTER(int destId, char direction, unsigned short x, unsigned short y, char hp);
bool SC_CREATE_OTHER_CHARACTER(int destId, int createId, char direction, unsigned short x, unsigned short y, char hp);
bool SC_DELETE_CHARACTER(int destId, int removeId);
bool SC_MOVE_START(int destId, int moveId, char moveDir, unsigned short x, unsigned short y);
bool SC_MOVE_STOP(int destId, int stopId, char viewDir, unsigned short x, unsigned short y);
bool SC_ATTACK1(int destId, int attackerId, char viewDir, unsigned short x, unsigned short y);
bool SC_ATTACK2(int destId, int attackerId, char viewDir, unsigned short x, unsigned short y);
bool SC_ATTACK3(int destId, int attackerId, char viewDir, unsigned short x, unsigned short y);
bool SC_DAMAGE(int destId, int attackerId, int victimId, char remainHp);

