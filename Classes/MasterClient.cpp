#include "MasterClient.h"

#include "NetMessage.h"

#include <algorithm>
#include <enet/time.h>

MasterClient::MasterClient(const std::string& masterIp, std::uint16_t masterPort)
{
	mMasterAdress = { ENET_HOST_ANY, ENET_PORT_ANY };
	enet_address_set_host(&mMasterAdress, masterIp.c_str());
	mMasterAdress.port = masterPort;
}

MasterClient::~MasterClient()
{
	if (mMasterSocket != ENET_SOCKET_NULL)
	{
		enet_socket_destroy(mMasterSocket);
		mMasterSocket = ENET_SOCKET_NULL;
	}

	if (mServerInfoSocket != ENET_SOCKET_NULL)
	{
		enet_socket_destroy(mServerInfoSocket);
		mServerInfoSocket = ENET_SOCKET_NULL;
	}
}

bool MasterClient::requestServerList()
{
	if (mMasterSocket == ENET_SOCKET_NULL)
	{
		mMasterSocket = enet_socket_create(ENET_SOCKET_TYPE_STREAM);
		if (mMasterSocket == ENET_SOCKET_NULL)
		{
			return false;
		}
	}

	if (enet_socket_connect(mMasterSocket, &mMasterAdress) == -1)
	{
		enet_socket_destroy(mMasterSocket);
		mMasterSocket = ENET_SOCKET_NULL;

		return false;
	}

	NetMessage msg;
	msg.writeUByte(LIST_SERVERS);

	sendMessage(msg);

	std::vector<char> data(256);

	mGameServers.clear();

	ENetBuffer buf;
	while (true)
	{
		enet_uint32 events = ENET_SOCKET_WAIT_RECEIVE;
		if (enet_socket_wait(mMasterSocket, &events, 250) >= 0 && events)
		{
			buf.data = data.data();
			buf.dataLength = data.size();
			int recv = enet_socket_receive(mMasterSocket, NULL, &buf, 1);
			if (recv <= 0)
				break;
			NetMessage msg;
			msg.write(data.data(), recv);
			while (!msg.isOnEnd())
			{
				std::uint32_t host;
				std::uint16_t port;

				msg.readUInt(host);
				msg.readUShort(port);

				GameServer server;
				server.address.host = host;
				server.address.port = port;
				server.infoPort = port + 1;
				mGameServers.push_back(server);
			}
			break;
		}
	}

	enet_socket_destroy(mMasterSocket);
	mMasterSocket = ENET_SOCKET_NULL;

	if (mOnServerListReady)
		mOnServerListReady(this);

	return true;
}

bool MasterClient::registerServer(enet_uint16 port)
{
	ENetAddress infoAddress;
	infoAddress.host = ENET_HOST_ANY;
	infoAddress.port = port + 1;
	mServerInfoSocket = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
	if (mServerInfoSocket == ENET_SOCKET_NULL)
	{
		return false;
	}
	if (enet_socket_bind(mServerInfoSocket, &infoAddress) < 0)
	{
		enet_socket_destroy(mServerInfoSocket);
		mServerInfoSocket = ENET_SOCKET_NULL;
		return false;
	}
	enet_socket_set_option(mServerInfoSocket, ENET_SOCKOPT_NONBLOCK, 1);

	if (mMasterSocket == ENET_SOCKET_NULL)
	{
		mMasterSocket = enet_socket_create(ENET_SOCKET_TYPE_STREAM);
		if (mMasterSocket == ENET_SOCKET_NULL)
		{
			return false;
		}

		if (enet_socket_connect(mMasterSocket, &mMasterAdress) == -1)
		{
			enet_socket_destroy(mMasterSocket);
			mMasterSocket = ENET_SOCKET_NULL;

			return false;
		}
	}

	NetMessage msg;
	msg.writeUByte(REG_SERVER);
	msg.writeUShort(port);

	sendMessage(msg);

	/*if (mMasterSocket != ENET_SOCKET_NULL)
	{
	enet_socket_destroy(mMasterSocket);
	mMasterSocket = ENET_SOCKET_NULL;
	}*/

	return true;
}

bool MasterClient::sendMessage(NetMessage& msg)
{
	ENetBuffer buf;
	while (msg.mROffset < msg.mWOffset)
	{
		enet_uint32 events = ENET_SOCKET_WAIT_SEND;
		if (enet_socket_wait(mMasterSocket, &events, 250) >= 0 && events)
		{
			buf.data = msg.getData() + msg.mROffset;
			buf.dataLength = msg.mWOffset - msg.mROffset;

			int sent = enet_socket_send(mMasterSocket, NULL, &buf, 1);
			if (sent < 0)
				return false;

			msg.mROffset += sent;

			if (msg.isOnEnd())
				break;
		}
	}

	return true;
}

void MasterClient::update(float deltaTime)
{
	if (mServerInfoSocket != ENET_SOCKET_NULL)
	{
		ENetBuffer buf;
		std::uint8_t pong[240];

		buf.data = pong;
		buf.dataLength = sizeof(pong);

		ENetAddress address;
		int len = enet_socket_receive(mServerInfoSocket, &address, &buf, 1);
		if (len > 0 && len < 240)
		{
			if (mOnWriteServerInfo)
			{
				NetMessage msg;
				mOnWriteServerInfo(msg);

				ENetBuffer buf;
				buf.data = msg.getData();
				buf.dataLength = msg.getDataSize();
				enet_socket_send(mServerInfoSocket, &address, &buf, 1);
			}
		}
	}
}