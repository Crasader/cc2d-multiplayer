#ifndef __NETOBJECT_H__
#define __NETOBJECT_H__

#include <cstdint>

class NetworkManager;
class NetMessage;

class NetObject
{
	friend NetworkManager;
public:
	NetObject();
	NetObject(NetworkManager* network);
	virtual ~NetObject();

	void setNetworkSyncEnable(bool enable) { mNetworkSync = enable; }

	virtual void onNetDestroy() {}

	void sendNetEvent(std::uint8_t eventId);
	void sendNetEvent(std::uint8_t eventId, NetMessage& msg);
	virtual void onNetEvent(std::uint8_t eventId) {}
	virtual void onNetEvent(std::uint8_t eventId, NetMessage& msg) {}

	virtual std::uint32_t calcNetPriority(std::uint16_t clientId);

	virtual void writeNetCreateInit(NetMessage& msg) {};
	virtual bool serialize(NetMessage& msg);

	std::uint32_t mNetId;
	std::uint32_t mTypeId;
	bool mNetworkSync = false;
	std::uint32_t mNetPriority = 0;

	NetworkManager* mNetwork;

private:
	void _notifyNetwork(NetworkManager* mgr) { mNetwork = mgr; }
};

#endif