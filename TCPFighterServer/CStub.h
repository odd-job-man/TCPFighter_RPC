#pragma once
#include "SerializeBuffer.h"
extern SerializeBuffer g_sb;
#pragma warning(disable : 4700)

class CStub
{
private:
	enum PacketNum
	{
		dfPACKET_CS_MOVE_START = 10,
		dfPACKET_CS_MOVE_STOP = 12,
		dfPACKET_CS_ATTACK1 = 20,
		dfPACKET_CS_ATTACK2 = 22,
		dfPACKET_CS_ATTACK3 = 24,
	};

public:
	bool PacketProc(int fromId, char packetType)
	{
		switch(packetType)
		{
		case dfPACKET_CS_MOVE_START:
			return CS_MOVE_START_RECV(fromId);
		case dfPACKET_CS_MOVE_STOP:
			return CS_MOVE_STOP_RECV(fromId);
		case dfPACKET_CS_ATTACK1:
			return CS_ATTACK1_RECV(fromId);
		case dfPACKET_CS_ATTACK2:
			return CS_ATTACK2_RECV(fromId);
		case dfPACKET_CS_ATTACK3:
			return CS_ATTACK3_RECV(fromId);
		default:
			return false;
		}
	}
	
	bool CS_MOVE_START_RECV(int fromId)
	{
		char moveDir;
		unsigned short x;
		unsigned short y;
		g_sb >> moveDir >> x >> y;
		g_sb.Clear();
		CS_MOVE_START(fromId, moveDir, x, y);
		return true;
	}

	virtual bool CS_MOVE_START(int fromId, char moveDir, unsigned short x, unsigned short y)
	{
		return false;
	}

	bool CS_MOVE_STOP_RECV(int fromId)
	{
		char viewDir;
		unsigned short x;
		unsigned short y;
		g_sb >> viewDir >> x >> y;
		g_sb.Clear();
		CS_MOVE_STOP(fromId, viewDir, x, y);
		return true;
	}

	virtual bool CS_MOVE_STOP(int fromId, char viewDir, unsigned short x, unsigned short y)
	{
		return false;
	}

	bool CS_ATTACK1_RECV(int fromId)
	{
		char viewDir;
		unsigned short x;
		unsigned short y;
		g_sb >> viewDir >> x >> y;
		g_sb.Clear();
		CS_ATTACK1(fromId, viewDir, x, y);
		return true;
	}

	virtual bool CS_ATTACK1(int fromId, char viewDir, unsigned short x, unsigned short y)
	{
		return false;
	}

	bool CS_ATTACK2_RECV(int fromId)
	{
		char viewDir;
		unsigned short x;
		unsigned short y;
		g_sb >> viewDir >> x >> y;
		g_sb.Clear();
		CS_ATTACK2(fromId, viewDir, x, y);
		return true;
	}

	virtual bool CS_ATTACK2(int fromId, char viewDir, unsigned short x, unsigned short y)
	{
		return false;
	}

	bool CS_ATTACK3_RECV(int fromId)
	{
		char viewDir;
		unsigned short x;
		unsigned short y;
		g_sb >> viewDir >> x >> y;
		g_sb.Clear();
		CS_ATTACK3(fromId, viewDir, x, y);
		return true;
	}

	virtual bool CS_ATTACK3(int fromId, char viewDir, unsigned short x, unsigned short y)
	{
		return false;
	}
};

class CSProc : public CStub
{
	virtual bool CS_MOVE_START(int fromId, char moveDir, unsigned short x, unsigned short y);
	virtual bool CS_MOVE_STOP(int fromId, char viewDir, unsigned short x, unsigned short y);
	virtual bool CS_ATTACK1(int fromId, char viewDir, unsigned short x, unsigned short y);
	virtual bool CS_ATTACK2(int fromId, char viewDir, unsigned short x, unsigned short y);
	virtual bool CS_ATTACK3(int fromId, char viewDir, unsigned short x, unsigned short y);
};

extern CSProc g_CSProc;

#pragma warning(default : 4700)