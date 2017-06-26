#include "NetObject.h"

#include "NetworkManager.h"

NetObject::NetObject()
	: mNetwork(nullptr)
	, mNetId(NetworkManager::InvalidNetId)
{
}

NetObject::NetObject(NetworkManager* network)
	: mNetwork(network)
	, mNetId(NetworkManager::InvalidNetId)
{
}

NetObject::~NetObject()
{
	if (mNetwork)
		mNetwork->unregisterObject(this);
}

void NetObject::sendNetEvent(std::uint8_t eventId)
{
	if (mNetId == NetworkManager::InvalidNetId)
		return;

	std::uint32_t time = 0;

	NetMessage msg(NetMessage::MSG_ObjectEvent);
	msg.writeUInt(mNetId);
	msg.writeUByte(eventId);
	msg.writeUInt(time);
	if (mNetwork)
		mNetwork->send(msg, ENET_PACKET_FLAG_RELIABLE);
}

void NetObject::sendNetEvent(std::uint8_t eventId, NetMessage& msg)
{
	if (mNetId == NetworkManager::InvalidNetId)
		return;

	std::uint32_t time = 0;

	NetMessage newMsg(NetMessage::MSG_ObjectEvent2);
	newMsg.writeUInt(mNetId);
	newMsg.writeUByte(eventId);
	newMsg.writeUInt(time);

	std::uint16_t dataSize = msg.getDataSize();
	newMsg.writeUShort(dataSize);
	newMsg.write(msg.getData(), dataSize);

	if (mNetwork)
		mNetwork->send(newMsg, ENET_PACKET_FLAG_RELIABLE);
}

std::uint32_t NetObject::calcNetPriority(std::uint16_t clientId)
{
	return NET_MAX_PRIORITY;
}

bool NetObject::serialize(NetMessage& msg)
{
	if (msg.mMsgId == NetMessage::MSG_CreateObject)
	{
		return true;
	}

	return false;
}