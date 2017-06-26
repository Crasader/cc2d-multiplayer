#include "MasterServer.h"

#include <iostream>
#include <cstdint>

#include <enet/time.h>

MasterServer::MasterServer()
{
	if (enet_initialize() != 0)
	{
		std::cout << "ENet initializing failed." << std::endl;
		return;
	}
}

MasterServer::~MasterServer()
{
	enet_deinitialize();
}

bool MasterServer::start()
{
	int port = 12345;

	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = port;

	mPingSocket = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
	if (enet_socket_bind(mPingSocket, &address) < 0)
	{
		return false;
	}
	if (enet_socket_set_option(mPingSocket, ENET_SOCKOPT_NONBLOCK, 1) < 0)
	{
		return false;
	}

	mServerSocket = enet_socket_create(ENET_SOCKET_TYPE_STREAM);
	if (mServerSocket == ENET_SOCKET_NULL ||
		enet_socket_set_option(mServerSocket, ENET_SOCKOPT_REUSEADDR, 1) < 0 ||
		enet_socket_bind(mServerSocket, &address) < 0 ||
		enet_socket_listen(mServerSocket, -1) < 0)
	{
		std::cout << "Failed to create socket." << std::endl;
		return false;
	}

	if (enet_socket_set_option(mServerSocket, ENET_SOCKOPT_NONBLOCK, 1) < 0)
	{
		std::cout << "Failed to make socket non-blocking." << std::endl;
		return false;
	}

	enet_time_set(0);

	std::cout << "Starting master server on Port: " << port << "." << std::endl;

	return true;
}

MasterServer::Client* MasterServer::addClient(const ENetAddress& address, ENetSocket socket)
{
	Client* client = new Client();
	client->address = address;
	client->socket = socket;
	client->lastInput = mTime;

	mClients.push_back(client);

	return client;
}

void MasterServer::removeClient(Client* client)
{
	for (size_t i = 0; i < mClients.size(); ++i)
	{
		if (mClients[i] == client)
		{
			removeClient(i);
			return;
		}
	}
}

void MasterServer::removeClient(size_t index)
{
	Client* client = mClients[index];

	enet_socket_destroy(client->socket);

	mClients[index] = mClients.back();
	mClients.pop_back();
	delete client;
}

void MasterServer::addGameServer(const ENetAddress& address, enet_uint16 port)
{
	char hn[260];
	if (enet_address_get_host_ip(&address, hn, sizeof(hn)) == 0)
	{
		std::string hostName = hn;

		std::cout << "added server " << hostName.c_str() << " Port: " << port << std::endl;

		GameServer server;
		server.address.host = address.host;
		server.address.port = port + 1;
		server.port = port;
		server.ip = hostName;
		server.lastPong = mTime;

		mGameServers.push_back(server);
	}
}

void MasterServer::removeGameServer(size_t index)
{
	mGameServers[index] = mGameServers.back();
	mGameServers.pop_back();
}

MasterServer::GameServer* MasterServer::findGameServer(enet_uint32 host, enet_uint16 port)
{
	for (size_t i = 0; i < mGameServers.size(); ++i)
	{
		if (mGameServers[i].address.host == host && mGameServers[i].port == port)
		{
			return &mGameServers[i];
		}
	}

	return nullptr;
}

MasterServer::GameServer* MasterServer::findGameServerByPingPort(enet_uint32 host, enet_uint16 port)
{
	for (size_t i = 0; i < mGameServers.size(); ++i)
	{
		if (mGameServers[i].address.host == host && mGameServers[i].address.port == port)
		{
			return &mGameServers[i];
		}
	}

	return nullptr;
}

void MasterServer::checkGameServers()
{
	ENetBuffer buf;
	ENetAddress addr;

	auto it = mGameServers.begin();
	while (it != mGameServers.end())
	{
		auto& server = *it;

		if (ENET_TIME_DIFFERENCE(mTime, server.lastPong) > 60000)
		{
			std::string ip = server.ip;

			it = mGameServers.erase(it);

			std::cout << "Server " + ip + " time out." << std::endl;
		}
		else
		{
			if (ENET_TIME_DIFFERENCE(mTime, server.lastPing) >= 3000)
			{
				enet_uint8 data[] = { 0 };
				buf.data = data;
				buf.dataLength = sizeof(data);
				enet_socket_send(mPingSocket, &server.address, &buf, 1);

				server.lastPing = mTime;
			}

			++it;
		}
	}

	{
		enet_uint8 data[120];
		buf.data = data;
		buf.dataLength = sizeof(data);
		int len = enet_socket_receive(mPingSocket, &addr, &buf, 1);
		if (len > 0)
		{
			GameServer* gameServer = findGameServerByPingPort(addr.host, addr.port);
			if (gameServer)
			{
				gameServer->lastPong = mTime;
			}
		}
	}
}

void MasterServer::run()
{
	while (true)
	{
		mTime = enet_time_get();

		{
			ENetAddress address;
			ENetSocket clientSocket = enet_socket_accept(mServerSocket, &address);
			if (clientSocket != ENET_SOCKET_NULL)
			{
				std::cout << "Client accepted";
				char hn[260];
				if (enet_address_get_host_ip(&address, hn, sizeof(hn)) == 0)
				{
					std::cout << " " << hn;
				}
				std::cout << std::endl;

				addClient(address, clientSocket);
			}
		}

		for (size_t i = 0; i < mClients.size(); ++i)
		{
			Client* client = mClients[i];
			{
				ENetBuffer buf;
				buf.data = &client->data[client->dataOffset];
				buf.dataLength = sizeof(client->data) - client->dataOffset;

				int res = enet_socket_receive(client->socket, NULL, &buf, 1);
				if (res > 0)
				{
					enet_uint32 packetSize = client->dataOffset + res;

					std::uint8_t type = 0;
					memcpy(&type, &client->data[0] + client->dataOffset, sizeof(uint8_t));
					client->dataOffset += sizeof(uint8_t);

					enet_uint32 packetSize1 = sizeof(uint8_t);
					enet_uint32 packetSize2 = sizeof(uint8_t) + sizeof(uint16_t);

					if (type == 1 && packetSize >= packetSize1)
					{
						client->dataOffset = 0;

						client->readOffset = 0;
						client->writeOffset = 0;

						for (auto gameServer : mGameServers)
						{
							{
								const unsigned size = sizeof(enet_uint32);
								char* data = (char*)&gameServer.address.host;

								std::size_t start = client->buffer.size();
								client->buffer.resize(start + size);
								memcpy(&client->buffer[0] + start, data, size);
								client->writeOffset = start + size;
							}
							{
								const unsigned size = sizeof(enet_uint16);
								char* data = (char*)&gameServer.port;

								std::size_t start = client->buffer.size();
								client->buffer.resize(start + size);
								memcpy(&client->buffer[0] + start, data, size);
								client->writeOffset = start + size;
							}
						}
					}
					else if (type == 0 && packetSize >= packetSize2)
					{
						uint16_t port = 0;
						memcpy(&port, &client->data[0] + client->dataOffset, sizeof(uint16_t));
						client->dataOffset += sizeof(uint16_t);

						client->dataOffset = 0;

						GameServer* gameServer = findGameServer(client->address.host, port);
						if (gameServer)
						{
							gameServer->lastPing = 0;
						}
						else
						{
							addGameServer(client->address, port);
						}
					}
				}
				else if (res < 0)
				{
					removeClient(i);

					continue;
				}

				if (client->buffer.size() > 0 && client->readOffset < client->writeOffset)
				{
					ENetBuffer buf;
					buf.data = &client->buffer[client->readOffset];
					buf.dataLength = client->writeOffset - client->readOffset;
					int res = enet_socket_send(client->socket, NULL, &buf, 1);
					if (res > 0)
					{
						client->readOffset += res;
						if (client->readOffset >= client->writeOffset)
						{
							client->buffer.clear();
							client->readOffset = 0;
							client->writeOffset = 0;
						}
					}
					else if (res < 0)
					{
						removeClient(i);

						continue;
					}
				}

				if (ENET_TIME_DIFFERENCE(mTime, client->lastInput) > 4000)
				{
					removeClient(i);

					continue;
				}
			}
		}

		checkGameServers();

		Sleep(50);
	}
}