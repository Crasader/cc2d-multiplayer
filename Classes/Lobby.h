#ifndef __LOBBY_H__
#define __LOBBY_H__

#include <cstdint>
#include <string>
#include <vector>

#include "NetObject.h"

class NetworkManager;
class NetMessage;

class Lobby : public NetObject
{
public:
	struct LobbyUser
	{
		std::uint16_t clientId = 6000;
		std::uint16_t lobbyId = 6000;
		bool isConnected = false;
		std::string name;
		std::uint32_t score = 0;
		std::uint16_t latency = 0;
	};

public:
	Lobby();
	~Lobby();

	std::uint16_t addUser(std::uint16_t clientId, const std::string name);
	void addUser(size_t index, std::uint16_t clientId, const std::string name);
	void removeUser(std::uint16_t clientId);
	void removeUserAtIndex(size_t index);
	void removeAllUsers();

	LobbyUser* getUser(size_t index);
	LobbyUser* getUserByClientId(std::uint16_t clientId);

	void sendLobbyUserData();

	virtual std::uint32_t calcNetPriority(std::uint16_t clientId) override;
	virtual bool serialize(NetMessage& msg) override;

	std::vector<LobbyUser> mLobbyUsers;
};

#endif