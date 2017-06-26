#ifndef __NETSERVICE_H__
#define __NETSERVICE_H__

class NetMessage;
class NetworkManager;

class NetService
{
	friend NetworkManager;
public:
	virtual void onMessage(NetMessage& msg) {}
	virtual void update(float deltaTime) {}

protected:
	NetworkManager* mNetwork = nullptr;

private:
	void _notifyNetwork(NetworkManager* mgr) { mNetwork = mgr; }
};

#endif