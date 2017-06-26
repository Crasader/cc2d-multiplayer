#include "SearchLanHost.h"

#include <enet/time.h>

#include "NetMessage.h"

SearchLanHost::SearchLanHost(bool isHost, std::uint16_t port)
	: mIsHost(isHost)
	, mHostPort(port)
{
	if (mIsHost)
	{
		mSocket = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
		if (mSocket == ENET_SOCKET_NULL)
			return;

		if (enet_socket_set_option(mSocket, ENET_SOCKOPT_REUSEADDR, 1) < 0)
			return;

		ENetAddress address;
		address.host = ENET_HOST_ANY;
		address.port = 4400;
		if (enet_socket_bind(mSocket, &address) < 0)
			return;

		enet_socket_set_option(mSocket, ENET_SOCKOPT_NONBLOCK, 1);
	}
}

SearchLanHost::~SearchLanHost()
{
	abortSearch();
}

void SearchLanHost::startSearch(enet_uint32 timeout)
{
	if (mIsHost)
		return;

	mIsDone = false;

	mSearchTimeout = timeout;
	mStartTime = enet_time_get();
	mLastSendTime = 0;
}

void SearchLanHost::abortSearch()
{
	mIsDone = true;

	if (mSocket != ENET_SOCKET_NULL)
	{
		enet_socket_destroy(mSocket);
		mSocket = ENET_SOCKET_NULL;
	}
}

bool SearchLanHost::isDone() const
{
	return mIsDone;
}

void SearchLanHost::update(float deltaTime)
{
	if (isDone())
		return;

	if (mIsHost)
	{
		ENetBuffer buf;
		ENetAddress addr;

		const int maxDataSize = 256;
		char data[maxDataSize];

		while (true)
		{
			buf.data = data;
			buf.dataLength = sizeof(data);
			int received = enet_socket_receive(mSocket, &addr, &buf, 1);
			if (received <= 0)
				break;

			if (received == sizeof(std::uint8_t) && data[0] == MT_Search)
			{
				NetMessage sendMsg;
				sendMsg.writeUByte(MT_SearchResult);
				sendMsg.writeUShort(mHostPort);

				buf.data = sendMsg.getData();
				buf.dataLength = sendMsg.getDataSize();

				enet_socket_send(mSocket, &addr, &buf, 1);
			}
		}
	}
	else
	{
		enet_uint32 time = enet_time_get();

		if (!mLastSendTime || ENET_TIME_DIFFERENCE(time, mLastSendTime) >= (4000))
		{
			mLastSendTime = time;

			if (mSocket == ENET_SOCKET_NULL)
			{
				mSocket = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
				if (mSocket == ENET_SOCKET_NULL)
					return;

				enet_socket_set_option(mSocket, ENET_SOCKOPT_NONBLOCK, 1);
				enet_socket_set_option(mSocket, ENET_SOCKOPT_BROADCAST, 1);
			}

			ENetBuffer buf;
			ENetAddress address;

			address.host = ENET_HOST_BROADCAST;
			address.port = 4400;

			std::uint8_t data[] = { MT_Search };
			buf.data = data;
			buf.dataLength = sizeof(data);

			int len = enet_socket_send(mSocket, &address, &buf, 1);
		}

		const int maxDataSize = 2048;
		char data[maxDataSize];

		while (true)
		{
			ENetBuffer buf;
			ENetAddress addr;

			buf.data = data;
			buf.dataLength = sizeof(data);
			int received = enet_socket_receive(mSocket, &addr, &buf, 1);
			if (received <= 0)
				break;

			NetMessage msg;
			msg.write(data, received);
			std::uint8_t type = MT_None;
			msg.readUByte(type);

			if (type == MT_SearchResult)
			{
				std::uint16_t port = 2554;
				msg.readUShort(port);

				std::string hostName;
				char hn[260];
				if (enet_address_get_host_ip(&addr, hn, sizeof(hn)) == 0)
					hostName = hn;

				if (mOnFound)
					mOnFound(addr.host, port);
			}
		}

		if (ENET_TIME_DIFFERENCE(time, mStartTime) >= mSearchTimeout)
		{
			abortSearch();
		}
	}
}