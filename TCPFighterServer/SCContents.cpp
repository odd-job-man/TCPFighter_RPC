#include "SerializeBuffer.h"
#include "SCContents.h"
extern SerializeBuffer g_sb;

bool EnqPacketUnicast(const int id, char* pPacket, const size_t packetSize);

int MAKE_HEADER(unsigned char packetSize, unsigned char packetType)
{
	g_sb << (unsigned char)0x89 << (unsigned char)packetSize << (unsigned char)packetType;
	return sizeof((char)0x89) + sizeof(packetSize) + sizeof(packetType);
}

bool SC_CREATE_MY_CHARACTER(int destId, char direction, unsigned short x, unsigned short y, char hp)
{
	constexpr int size = sizeof(destId) + sizeof(direction) + sizeof(x) + sizeof(y) + sizeof(hp);
	int headerSize = MAKE_HEADER(size, dfPACKET_SC_CREATE_MY_CHARACTER);
	g_sb << destId << direction << x << y << hp;
	bool EnqRet = EnqPacketUnicast(destId, g_sb.GetBufferPtr(), headerSize + size);
	g_sb.Clear();
	return EnqRet;
}

bool SC_CREATE_OTHER_CHARACTER(int destId, int createId, char direction, unsigned short x, unsigned short y, char hp)
{
	constexpr int size = sizeof(createId) + sizeof(direction) + sizeof(x) + sizeof(y) + sizeof(hp);
	int headerSize = MAKE_HEADER(size, dfPACKET_SC_CREATE_OTHER_CHARACTER);
	g_sb << createId << direction << x << y << hp;
	bool EnqRet = EnqPacketUnicast(destId, g_sb.GetBufferPtr(), headerSize + size);
	g_sb.Clear();
	return EnqRet;
}

bool SC_DELETE_CHARACTER(int destId, int removeId)
{
	constexpr int size = sizeof(removeId);
	int headerSize = MAKE_HEADER(size, dfPACKET_SC_DELETE_CHARACTER);
	g_sb << removeId;
	bool EnqRet = EnqPacketUnicast(destId, g_sb.GetBufferPtr(), headerSize + size);
	g_sb.Clear();
	return EnqRet;
}

bool SC_MOVE_START(int destId, int moveId, char moveDir, unsigned short x, unsigned short y)
{
	constexpr int size = sizeof(moveId) + sizeof(moveDir) + sizeof(x) + sizeof(y);
	int headerSize = MAKE_HEADER(size, dfPACKET_SC_MOVE_START);
	g_sb << moveId << moveDir << x << y;
	bool EnqRet = EnqPacketUnicast(destId, g_sb.GetBufferPtr(), headerSize + size);
	g_sb.Clear();
	return EnqRet;
}

bool SC_MOVE_STOP(int destId, int stopId, char viewDir, unsigned short x, unsigned short y)
{
	constexpr int size = sizeof(stopId) + sizeof(viewDir) + sizeof(x) + sizeof(y);
	int headerSize = MAKE_HEADER(size, dfPACKET_SC_MOVE_STOP);
	g_sb << stopId << viewDir << x << y;
	bool EnqRet = EnqPacketUnicast(destId, g_sb.GetBufferPtr(), headerSize + size);
	g_sb.Clear();
	return EnqRet;
}

bool SC_ATTACK1(int destId, int attackerId, char viewDir, unsigned short x, unsigned short y)
{
	constexpr int size = sizeof(attackerId) + sizeof(viewDir) + sizeof(x) + sizeof(y);
	int headerSize = MAKE_HEADER(size, dfPACKET_SC_ATTACK1);
	g_sb << attackerId << viewDir << x << y;
	bool EnqRet = EnqPacketUnicast(destId, g_sb.GetBufferPtr(), headerSize + size);
	g_sb.Clear();
	return EnqRet;
}

bool SC_ATTACK2(int destId, int attackerId, char viewDir, unsigned short x, unsigned short y)
{
	constexpr int size = sizeof(attackerId) + sizeof(viewDir) + sizeof(x) + sizeof(y);
	int headerSize = MAKE_HEADER(size, dfPACKET_SC_ATTACK2);
	g_sb << attackerId << viewDir << x << y;
	bool EnqRet = EnqPacketUnicast(destId, g_sb.GetBufferPtr(), headerSize + size);
	g_sb.Clear();
	return EnqRet;
}

bool SC_ATTACK3(int destId, int attackerId, char viewDir, unsigned short x, unsigned short y)
{
	constexpr int size = sizeof(attackerId) + sizeof(viewDir) + sizeof(x) + sizeof(y);
	int headerSize = MAKE_HEADER(size, dfPACKET_SC_ATTACK3);
	g_sb << attackerId << viewDir << x << y;
	bool EnqRet = EnqPacketUnicast(destId, g_sb.GetBufferPtr(), headerSize + size);
	g_sb.Clear();
	return EnqRet;
}

bool SC_DAMAGE(int destId, int attackerId, int victimId, char remainHp)
{
	constexpr int size = sizeof(attackerId) + sizeof(victimId) + sizeof(remainHp);
	int headerSize = MAKE_HEADER(size, dfPACKET_SC_DAMAGE);
	g_sb << attackerId << victimId << remainHp;
	bool EnqRet = EnqPacketUnicast(destId, g_sb.GetBufferPtr(), headerSize + size);
	g_sb.Clear();
	return EnqRet;
}

