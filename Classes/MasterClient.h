#ifndef __MASTERSERVER_H__
#define __MASTERSERVER_H__

#include <enet/enet.h>

#include <functional>
#include <vector>
#include <cstdint>

#include "NetService.h"

class NetMessage;

class MasterClient : public NetService
{
public:
	enum
	{
		REG_SERVER = 0,
		LIST_SERVERS = 1,
	};

	struct GameServer
	{
		// ping address
		ENetAddress address;
		std::string ip;
		enet_uint16 infoPort;
	};

public:
	MasterClient(const std::string& masterIp, std::uint16_t masterPort);
	~MasterClient();

	bool requestServerList();

	bool registerServer(enet_uint16 port);

	bool sendMessage(NetMessage& msg);

	void update(float deltaTime) override;

	std::function<void(MasterClient*)> mOnServerListReady;
	std::function<void(NetMessage&)> mOnWriteServerInfo;

private:
	ENetAddress mMasterAdress;
	ENetSocket mMasterSocket = ENET_SOCKET_NULL;

	ENetSocket mServerInfoSocket = ENET_SOCKET_NULL;

public:
	std::vector<GameServer> mGameServers;
};

#endif