#ifndef __NETWORKMANAGER_H__
#define __NETWORKMANAGER_H__

#include <string>
#include <cstdint>
#include <memory>
#include <map>

#include <enet/enet.h>

#include "NetMessage.h"
#include "SnapshotManager.h"
#include "Delegate.h"

class SearchLanHost;
class NetObject;
class NetService;
class MasterClient;
class Parameters;
class Lobby;

#define NET_MAX_CLIENTS 4U
#define NET_MAX_PRIORITY 120U
#define NET_DEFAULT_PORT 1234

class NetworkManager
{
public:
	struct PlayerController
	{
		void reset()
		{
			player = nullptr;
			spectating = false;
			wantSpectate = false;
			spactatorId = NetworkManager::InvalidNetId;
			forceRespawn = true;
			team = -1;
		}

		bool wantRespawn()
		{
			return (forceRespawn && !wantSpectate && !player);
		}

		void onRespawn()
		{
			forceRespawn = true;
			spectating = false;
			wantSpectate = false;
			spactatorId = NetworkManager::InvalidNetId;
		}

		bool hasPlayer() const;

		NetObject* player = nullptr;

		int team = -1;
		bool spectating = false;
		bool wantSpectate = false;
		std::uint32_t spactatorId = NetworkManager::InvalidNetId;
		bool forceRespawn = true;
		std::uint32_t spectateStartTime = 0;
	};

	struct Connection
	{
		void reset()
		{
			peer = nullptr;
			isConnected = false;
			isReady = false;
			isMapLoadFinished = false;
			snapshotManager.reset();
			pendingSnapShot.clear();
			lastSnapTime = 0;

			gameTime = 0;

			snapRate = 100;

			controller.reset();

			netObjectPriority.clear();
		}

		std::uint16_t getClientId()
		{
			return peer->incomingPeerID;
		}

		std::uint32_t getPredictTime()
		{
			return enet_time_get() + peer->roundTripTime;
		}

		std::uint32_t getRoundTripTime()
		{
			return peer ? peer->roundTripTime : 0U;
		}

		std::uint32_t getpacketLoss()
		{
			return peer ? peer->packetLoss : 0U;
		}

		std::string name;
		std::string hostName;
		ENetPeer* peer = nullptr;

		bool isConnected = false;
		bool isReady = false;
		bool isMapLoadFinished = false;

		PlayerController controller;

		SnapshotManager snapshotManager;
		SnapShot pendingSnapShot;

		SnapShot lastSnapShot;

		std::uint32_t lastSnapTime = 0;
		std::uint32_t snapRate = 100;
		std::uint32_t gameTime = 0;

		std::map<std::uint32_t, std::uint32_t> netObjectPriority;
	};

	struct DestroyObject
	{
		DestroyObject(std::uint32_t _typeId, std::uint32_t _netId)
			: typeId(_typeId)
			, netId(_netId)
		{}

		std::uint32_t typeId;
		std::uint32_t netId;
	};

	static std::uint16_t InvalidClientId;
	static std::uint16_t ServerId;
	static std::uint32_t InvalidNetId;

public:
	NetworkManager();
	~NetworkManager();

	void setError(const std::string& error);

	void initConfig(Parameters& config);

	void resetState();

	ENetHost* getHost() const { return mHost; };
	ENetPeer* getPeer() const { return mPeer; }

	std::uint16_t getClientId() const { return mClientId; }

	std::uint32_t getSnapServerTime() const { return mSnapServerTime; }

	std::int32_t getSnapRate() const { return mSnapRate; }
	std::uint32_t getSnapCurrentTime() const { return mSnapCurrentTime; }
	std::uint32_t getInterpolationTime() const { return mInterpolationTime; }

	float getRelevantDistance() const { return mRelevantDistance; }

	float getTotalSendData() const { return mTotalSendData; }
	float getTotalReceivedData() const { return mTotalReceivedData; }

	float getSyncFrequency() const { return mSyncFrequency; }
	void setSyncFrequency(float freq) { mSyncFrequency = freq; }

	int getSnapshotTotalTime() const { return mSnapshotTotalTime; }

	std::uint32_t calcNumSnapsBehind();

	Connection* getConnection(std::uint32_t id);
	PlayerController* getPlayerController(std::uint32_t id);

	void setCurrentSnapSequence(std::uint32_t sequence);

	std::uint32_t getTime() const;
	std::uint32_t getServiceTime() const;
	std::uint32_t getCurrTime() const;

	bool registerService(NetService* service);
	bool unregisterService(NetService* service);
	void destroyService(NetService* service);
	template <class T>
	T* findService()
	{
		for (auto service : mServices)
		{
			T* t = dynamic_cast<T*>(service);
			if (t)
				return t;
		}
		return nullptr;
	}

	bool startServer(unsigned short port);
	bool connect(const std::string& remoteAddress, unsigned short remotePort);
	void disconnect();

	void disconnectClient(std::uint16_t id, std::uint32_t reason = 0);

	void kickClient(std::uint16_t id);

	bool isConnected() const;

	bool isServer() const;
	bool isClient() const;

	std::uint32_t getNumClients();
	bool isClientConnected(std::uint16_t id);

	std::uint32_t getFreeNetObjectId();
	bool isNetIdValid(std::uint32_t netId) const;

	void registerObject(NetObject* obj);
	void registerObject(NetObject* obj, std::uint32_t id);
	void unregisterObject(NetObject* obj);

	void registerStaticObject(std::uint32_t index, NetObject* obj);
	void setStaticObject(std::uint32_t index, NetObject* obj);

	void flushDestroyObjects();

	NetObject* getObject(std::uint32_t netId);

	void send(NetMessage& msg, _ENetPacketFlag packetFlag = ENET_PACKET_FLAG_RELIABLE, int channel = 0);
	void sendTo(const std::uint16_t clientId, NetMessage& msg, _ENetPacketFlag packetFlag = ENET_PACKET_FLAG_RELIABLE, int channel = 0);
	void sendNotTo(const std::uint16_t clientId, NetMessage& msg, _ENetPacketFlag packetFlag = ENET_PACKET_FLAG_RELIABLE, int channel = 0);

	void sendServerInfo(std::uint16_t clientId);
	void sendClientInfo();

	void sendMap(const std::string& name);
	void sendMapLoadFinished();

	void sendAllObjects(std::uint16_t clientId);

	void sendSpectating(std::uint32_t clientId, std::uint32_t spectator);
	void sendSpectateNext(std::uint16_t clientId, std::uint32_t spectator);

	void sendUserScore();

	void sendSnapShots();
	void sendClientSnapShots();

	std::uint32_t getClientLastReceiveTime(const std::uint16_t clientId);

	bool isClientLagged(const std::uint16_t clientId);
	bool isLagged();

	bool checkPlayerName(Connection* client);

	void update(float deltaTime);

public:
	MulticastDelegate<std::uint32_t> mOnConnect;
	MulticastDelegate<std::uint32_t> mOnDisconnect;
	MulticastDelegate<NetMessage&> mOnMessage;

	std::function<NetObject*(std::uint32_t, NetMessage& msg)> mOnHandleCreateObject;
	std::function<std::uint32_t(std::uint32_t, NetMessage& msg)> mOnSerializeStaticObjects;
	Delegate<> mOnAllClientsLoadingFinished;
	Delegate<NetMessage&> mOnWriteMapInfo;
	Delegate<NetMessage&> mOnWriteServerInfo;
	Delegate<const std::string&> mOnErrorMessage;

private:
	void handleMessage(NetMessage& msg);

	void handleSnapShot(NetMessage& msg, const std::uint8_t msgId, const std::uint16_t clientId);
	void handleAckSnapShot(NetMessage& msg, const std::uint8_t msgId, const std::uint16_t clientId);
	void handleCreateObject(NetMessage& msg, const std::uint8_t msgId, const std::uint16_t clientId);
	void handleDestroyObject(NetMessage& msg, const std::uint8_t msgId, const std::uint16_t clientId);
	void handleObjectEvent(NetMessage& msg, const std::uint8_t msgId, const std::uint16_t clientId);
	void handleClientInfo(NetMessage& msg, const std::uint8_t msgId, const std::uint16_t clientId);
	void handleMapChange(NetMessage& msg, const std::uint8_t msgId, const std::uint16_t clientId);

	void computeSnapShotTime();
	int calcSnapshotTotalTime();

	void onWriteServerInfo(NetMessage& msg);

private:
	ENetHost* mHost = nullptr;
	ENetPeer* mPeer = nullptr;

	float mSyncTime = 0.f;
	float mSyncFrequency;

	std::vector<NetObject*> mNetObjects;
	std::vector<DestroyObject> mNetDestroyObjects;

	std::vector<NetObject*> mStaticNetObjects;

	std::vector<NetService*> mServices;

	bool mIsServer = false;

	SnapshotManager mSnapshotManager;
	SnapShot mSnapShot;
	std::uint32_t mCurrSnapshotSeq = 1;
	
public:
	std::string mServerName = "Server";
	std::string mPlayerName;

	std::uint16_t mClientId = InvalidClientId;

	std::uint32_t mTime = 0;

	std::vector<std::unique_ptr<Connection>> mConnections;
	std::uint32_t mMaxConnections = NET_MAX_CLIENTS;

	std::uint32_t mMaxPlayers = 4;

	std::uint32_t mSnapServerTime = 0;

	std::int32_t mSnapRate = 1000;
	std::uint32_t mSnapCurrentTime = 0;
	std::uint32_t mInterpolationTime = 0;

	float mRelevantDistance = 40.f;

	NetObject* mLocalPlayer = nullptr;

	float mTotalSendData;
	float mTotalReceivedData;

private:
	std::vector<std::uint32_t> mSnapTimeHistory;
	std::uint32_t mSnapShotTimeHead = 0;
	std::uint32_t mSnapShotTimeTail = 0;

	std::uint32_t mCurrSnapServerTime = 0;
	std::uint32_t mPrevSnapServerTime = 0;

	int mSnapshotTotalTime;

	PlayerController mServerPlayerController;

	std::uint32_t mLastStatsTime = 0;

public:
	std::unique_ptr<SearchLanHost> mSearchLanHost;
	std::unique_ptr<MasterClient> mMasterClient;

	std::unique_ptr<Lobby> mLobby;

	std::string mMasterServerIp;
	std::uint16_t mMasterServerPort;

	bool mIsGameRunning = false;

	bool mSendFullSnapShot = false;
	float mDebugLerp = 0;
};

#endif