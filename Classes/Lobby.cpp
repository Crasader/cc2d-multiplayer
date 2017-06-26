#include "Lobby.h"

#include "NetworkManager.h"
#include "NetMessage.h"

Lobby::Lobby()
{
}

Lobby::~Lobby()
{

}

std::uint16_t Lobby::addUser(std::uint16_t clientId, const std::string name)
{
	std::uint16_t freeLobbyId = 0;
	for (size_t i = 0; i < mLobbyUsers.size(); ++i)
	{
		if (!mLobbyUsers[i].isConnected)
		{
			freeLobbyId = i;
			break;
		}
	}

	addUser(freeLobbyId, clientId, name);

	return freeLobbyId;
}

void Lobby::addUser(size_t index, std::uint16_t clientId, const std::string name)
{
	mLobbyUsers[index].clientId = clientId;
	mLobbyUsers[index].lobbyId = index;
	mLobbyUsers[index].isConnected = true;
	mLobbyUsers[index].name = name;
}

void Lobby::removeUser(std::uint16_t clientId)
{
	for (size_t i = 0; i < mLobbyUsers.size(); ++i)
	{
		if (mLobbyUsers[i].isConnected && mLobbyUsers[i].clientId == clientId)
		{
			removeUserAtIndex(i);
			break;
		}
	}
}

void Lobby::removeUserAtIndex(size_t index)
{
	mLobbyUsers[index].isConnected = false;
}

void Lobby::removeAllUsers()
{
	for (size_t i = 0; i < mLobbyUsers.size(); ++i)
	{
		mLobbyUsers[i].isConnected = false;
	}
}

Lobby::LobbyUser* Lobby::getUser(size_t index)
{
	if (index >= mLobbyUsers.size())
		return nullptr;

	return &mLobbyUsers[index];
}

Lobby::LobbyUser* Lobby::getUserByClientId(std::uint16_t clientId)
{
	for (size_t i = 0; i < mLobbyUsers.size(); ++i)
	{
		if (mLobbyUsers[i].isConnected && mLobbyUsers[i].clientId == clientId)
			return &mLobbyUsers[i];
	}

	return nullptr;
}

void Lobby::sendLobbyUserData()
{
	if (!mNetwork->isServer())
		return;

	NetMessage msg(NetMessage::MSG_Lobby_UserData);

	std::uint16_t numUsers = mNetwork->mMaxConnections;
	msg.writeUShort(numUsers);

	for (size_t i = 0; i < numUsers; ++i)
	{
		bool isConnected = mLobbyUsers[i].isConnected;

		msg.writeBool(isConnected);

		if (isConnected)
		{
			msg.writeUShort(mLobbyUsers[i].clientId);
			msg.writeString(mLobbyUsers[i].name);
		}
	}

	mNetwork->send(msg);
}

std::uint32_t Lobby::calcNetPriority(std::uint16_t clientId)
{
	return NET_MAX_PRIORITY / 12U;
}

bool Lobby::serialize(NetMessage& msg)
{
	switch (msg.mMsgId)
	{
	case NetMessage::MSG_Sync:
		{
			for (std::uint32_t i = 0U; i < NET_MAX_CLIENTS; i++)
			{
				std::uint16_t score = 0;
				std::uint16_t latency = 0;

				if (i < mLobbyUsers.size() && mLobbyUsers[i].isConnected)
				{
					auto client = mNetwork->getConnection(mLobbyUsers[i].clientId);
					if (client)
					{
						mLobbyUsers[i].latency = client->getRoundTripTime();
					}

					score = mLobbyUsers[i].score;
					latency = mLobbyUsers[i].latency;
				}

				msg.serializeUShort(score);
				msg.serializeUShort(latency);

				if (msg.isReading())
				{
					if (i < mLobbyUsers.size())
					{
						mLobbyUsers[i].score = score;
						mLobbyUsers[i].latency = latency;
					}
				}
			}

			break;
		}
	}

	return true;
}