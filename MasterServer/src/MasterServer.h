#ifndef __MASTERSERVER_H__
#define __MASTERSERVER_H__

#include <enet/enet.h>

#include <string>
#include <vector>

class MasterServer
{
public:
	struct GameServer
	{
		// ping address
		ENetAddress address;
		std::string ip;
		enet_uint16 port;
		enet_uint32 lastPing = 0;
		enet_uint32 lastPong = 0;
	};

	struct Client
	{
		ENetAddress address;
		ENetSocket socket;
		char data[256];
		enet_uint32 dataOffset = 0;
		enet_uint32 lastInput = 0;

		std::vector<char> buffer;
		size_t readOffset = 0;
		size_t writeOffset = 0;
	};

public:
	MasterServer();
	~MasterServer();

	bool start();

	Client* addClient(const ENetAddress& address, ENetSocket socket);
	void removeClient(Client* client);
	void removeClient(size_t index);

	void addGameServer(const ENetAddress& address, enet_uint16 port);
	void removeGameServer(size_t index);

	GameServer* findGameServer(enet_uint32 host, enet_uint16 port);
	GameServer* findGameServerByPingPort(enet_uint32 host, enet_uint16 port);

	void checkGameServers();

	void run();

	std::vector<GameServer> mGameServers;
	std::vector<Client*> mClients;

	enet_uint32 mTime = 0;

	ENetSocket mServerSocket = ENET_SOCKET_NULL;
	ENetSocket mPingSocket = ENET_SOCKET_NULL;
};

#endif