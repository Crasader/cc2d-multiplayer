#include "NetworkManager.h"

#include "VariableDeltaManager.h"
#include "NetService.h"

#include "MasterClient.h"
#include "SearchLanHost.h"
#include "Lobby.h"
#include "Parameters.h"

#include <sstream>
#include <algorithm>
#include <limits>

std::uint16_t NetworkManager::InvalidClientId = (std::numeric_limits<std::uint16_t>::max)();
std::uint16_t NetworkManager::ServerId = ENET_PROTOCOL_MAXIMUM_PEER_ID;
std::uint32_t NetworkManager::InvalidNetId = (std::numeric_limits<std::uint32_t>::max)();

bool NetworkManager::PlayerController::hasPlayer() const
{
	return (player != nullptr);
}

NetworkManager::NetworkManager()
	: mSyncFrequency(1.f / 50.f)
{
	if (enet_initialize() != 0)
	{
		setError("ENet initializing failed.");
		return;
	}

	enet_time_set(0);

	mLobby = std::make_unique<Lobby>();
	registerStaticObject(0, mLobby.get());
}

NetworkManager::~NetworkManager()
{
	for (auto service : mServices)
	{
		delete service;
	}
	mServices.clear();

	disconnect();

	enet_deinitialize();
}

void NetworkManager::setError(const std::string& error)
{
	mOnErrorMessage(error.c_str());
}

void NetworkManager::initConfig(Parameters& config)
{
	mMasterServerIp = config.getString("masterServerIp", "");
	mMasterServerPort = config.getInt("masterServerPort", 12345);
}

void NetworkManager::resetState()
{
	mSyncTime = 0;

	mInterpolationTime = 0;

	//mSnapShot.sequence = 0;
	mSnapShot.clear();

	mServerPlayerController.reset();

	for (size_t i = 0; i < mConnections.size(); ++i)
	{
		if (mConnections[i])
		{
			mConnections[i]->isMapLoadFinished = false;
			mConnections[i]->snapshotManager.reset();
			mConnections[i]->pendingSnapShot.clear();
			mConnections[i]->lastSnapShot.clear();
			mConnections[i]->lastSnapTime = 0;
			mConnections[i]->snapRate = 0;
			mConnections[i]->gameTime = 0;

			mConnections[i]->controller.reset();
		}
	}

	mSnapShotTimeHead = 0;
	mSnapShotTimeTail = 0;

	mCurrSnapServerTime = 0;
	mPrevSnapServerTime = 0;

	mSnapCurrentTime = 0;

	if (isClient())
		mSnapTimeHistory.resize(16);

	mSnapshotManager.reset();
}

std::uint32_t NetworkManager::calcNumSnapsBehind()
{
	return mSnapShotTimeHead > mSnapShotTimeTail ? mSnapTimeHistory.size() - mSnapShotTimeHead + mSnapShotTimeTail : mSnapShotTimeTail - mSnapShotTimeHead;
}

NetworkManager::Connection* NetworkManager::getConnection(std::uint32_t id)
{
	for (size_t i = 0; i < mConnections.size(); ++i)
	{
		if (mConnections[i] && mConnections[i]->isConnected && mConnections[i]->peer && mConnections[i]->peer->incomingPeerID == id)
		{
			return mConnections[i].get();
		}
	}
	return nullptr;
}

NetworkManager::PlayerController* NetworkManager::getPlayerController(std::uint32_t id)
{
	if (id == ServerId)
	{
		return &mServerPlayerController;
	}

	Connection* client = getConnection(id);
	if (client && client->isConnected && client->isReady)
	{
		return &client->controller;
	}
	return nullptr;
}

void NetworkManager::setCurrentSnapSequence(std::uint32_t sequence)
{
	if (isClient())
	{
		mSnapShot.sequence = sequence;
	}
}

std::uint32_t NetworkManager::getTime() const
{
	return mTime;
}

std::uint32_t NetworkManager::getServiceTime() const
{
	return (mHost ? mHost->serviceTime : 0U);
}

std::uint32_t NetworkManager::getCurrTime() const
{
	return enet_time_get();
}

bool NetworkManager::registerService(NetService* service)
{
	if (!service)
		return false;

	auto it = std::find(mServices.begin(), mServices.end(), service);
	if (it == mServices.end())
	{
		service->_notifyNetwork(this);
		mServices.push_back(service);
		return true;
	}
	return false;
}

bool NetworkManager::unregisterService(NetService* service)
{
	auto it = std::find(mServices.begin(), mServices.end(), service);
	if (it != mServices.end())
	{
		mServices.erase(it);

		return true;
	}
	return false;
}

void NetworkManager::destroyService(NetService* service)
{
	auto it = std::find(mServices.begin(), mServices.end(), service);
	if (it != mServices.end())
	{
		mServices.erase(it);
	}

	if (service)
	{
		delete service;
	}
}

bool NetworkManager::startServer(unsigned short port)
{
	disconnect();

	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = port;

	//ENetChannels = 3; // 0 = Sequenced, 1 = Reliable, 2 = Unsequenced
	mHost = enet_host_create(&address      /* the address to bind the server host to */,
		mMaxConnections/* number of connections */,
		3              /* channel limit */,
		0              /* incoming bandwidth */,
		0              /* outgoing bandwidth */);
	if (!mHost)
	{
		setError("enet_host_create");
		return false;
	}

	mIsServer = true;
	mClientId = ServerId;

	mMasterClient = std::make_unique<MasterClient>(mMasterServerIp, mMasterServerPort);
	mMasterClient->mOnWriteServerInfo = std::bind(&NetworkManager::onWriteServerInfo, this, std::placeholders::_1);
	mMasterClient->registerServer(port);

	mSearchLanHost = std::make_unique<SearchLanHost>(true, port);

	return true;
}

bool NetworkManager::connect(const std::string& remoteAddress, unsigned short remotePort)
{
	disconnect();

	mIsServer = false;

	mHost = enet_host_create(NULL /* create a client host */,
		1    /* only allow 1 outgoing connection */,
		3    /* channel limit */,
		128000 / 8    /* downstream bandwidth */,
		56000 / 8    /*  upstream bandwidth   */);
	if (!mHost)
	{
		setError("enet_host_create");
		return false;
	}

	ENetAddress address;
	enet_address_set_host(&address, remoteAddress.c_str());
	address.port = remotePort;

	mPeer = enet_host_connect(mHost, &address, 3, 0);
	if (!mPeer)
	{
		enet_host_destroy(mHost);
		mHost = nullptr;

		setError("enet_host_connect");
		return false;
	}

	/* Wait up to 5 seconds for the connection attempt to succeed. */
	ENetEvent event;
	if (enet_host_service(mHost, &event, 5000) <= 0 ||
		event.type != ENET_EVENT_TYPE_CONNECT)
	{
		enet_peer_reset(mPeer);
		mPeer = nullptr;
		enet_host_destroy(mHost);
		mHost = nullptr;

		setError("wait for connect");
		return false;
	}

	enet_peer_throttle_configure(mPeer, 5000, 2, 2);

	mClientId = mPeer->outgoingPeerID;

	return true;
}

void NetworkManager::disconnect()
{
	if (mPeer)
	{
		std::uint32_t reason = 0;
		enet_peer_disconnect(mPeer, reason);

		/* Allow up to 3 seconds for the disconnect to succeed
		* and drop any packets received packets.
		*/
		ENetEvent event;
		while (enet_host_service(mHost, &event, 3000) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_RECEIVE:
				enet_packet_destroy(event.packet);
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				mPeer = nullptr;
				break;
			}
		}

		if (mPeer)
		{
			/* We've arrived here, so the disconnect attempt didn't */
			/* succeed yet.  Force the connection down.             */
			enet_peer_reset(mPeer);
			mPeer = nullptr;
		}
	}

	if (mHost)
	{
		if (isServer())
		{
			for (size_t i = 0; i < mHost->peerCount; ++i)
			{
				disconnectClient(mHost->peers[i].incomingPeerID);
			}

			enet_host_flush(mHost);

			mMasterClient.reset();
			mSearchLanHost.reset();
		}

		enet_host_destroy(mHost);
		mHost = nullptr;
	}

	mIsServer = false;
}

void NetworkManager::disconnectClient(std::uint16_t id, std::uint32_t reason)
{
	Connection* client = getConnection(id);
	if (!client)
		return;

	enet_peer_disconnect(client->peer, reason);

	client->isConnected = false;
}

void NetworkManager::kickClient(std::uint16_t id)
{
	disconnectClient(id);
}

bool NetworkManager::isConnected() const
{
	return (mHost != nullptr);
}

bool NetworkManager::isServer() const
{
	return mIsServer;
}

bool NetworkManager::isClient() const
{
	return mHost && mPeer && !isServer();
}

std::uint32_t NetworkManager::getNumClients()
{
	std::uint32_t num = 0;
	for (size_t i = 0; i < mConnections.size(); ++i)
	{
		if (mConnections[i] && mConnections[i]->isConnected)
			num++;
	}
	return num;
}

bool NetworkManager::isClientConnected(std::uint16_t id)
{
	Connection* client = getConnection(id);
	if (client)
	{
		if (client->isConnected)
			return true;
	}
	return false;
}

std::uint32_t NetworkManager::getFreeNetObjectId()
{
	for (size_t i = 0; i < mNetObjects.size(); ++i)
	{
		if (!mNetObjects[i])
		{
			return i;
		}
	}
	size_t newId = mNetObjects.size();
	mNetObjects.push_back(nullptr);
	return newId;
}

bool NetworkManager::isNetIdValid(std::uint32_t netId) const
{
	return (netId < mNetObjects.size());
}

void NetworkManager::registerObject(NetObject* obj)
{
	if (isServer() && obj && obj->mNetId == InvalidNetId)
	{
		flushDestroyObjects();

		obj->_notifyNetwork(this);

		obj->mNetId = getFreeNetObjectId();
		mNetObjects[obj->mNetId] = obj;

		NetMessage msg(NetMessage::MSG_CreateObject);
		msg.writeUShort(1);
		msg.writeUInt(obj->mTypeId);
		msg.writeUInt(obj->mNetId);
		obj->writeNetCreateInit(msg);
		if (obj->serialize(msg))
		{
			for (size_t i = 0; i < mConnections.size(); ++i)
			{
				if (mConnections[i] && mConnections[i]->isConnected && mConnections[i]->isMapLoadFinished)
				{
					sendTo(mConnections[i]->getClientId(), msg, ENET_PACKET_FLAG_RELIABLE);
				}
			}
		}
	}
}

void NetworkManager::registerObject(NetObject* obj, std::uint32_t id)
{
	if (obj && obj->mNetId == InvalidNetId)
	{
		if (id >= mNetObjects.size())
		{
			mNetObjects.resize(id + 1);
		}
		obj->mNetId = id;
		mNetObjects[id] = obj;

		obj->_notifyNetwork(this);
	}
	else
	{
		std::stringstream ss;
		ss << id;
		setError("registerObject: " + ss.str());
	}
}

void NetworkManager::unregisterObject(NetObject* obj)
{
	if (obj && obj->mNetId != InvalidNetId)
	{
		if (isServer())
		{
			mNetDestroyObjects.emplace_back(obj->mTypeId, obj->mNetId);
		}

		if (isNetIdValid(obj->mNetId))
		{
			mNetObjects[obj->mNetId] = nullptr;

			for (size_t i = 0; i < mConnections.size(); ++i)
			{
				if (mConnections[i] && mConnections[i]->isConnected && mConnections[i]->isReady && mConnections[i]->isMapLoadFinished)
				{
					auto it = mConnections[i]->netObjectPriority.find(obj->mNetId + mStaticNetObjects.size());
					if (it != mConnections[i]->netObjectPriority.end())
						mConnections[i]->netObjectPriority.erase(it);
				}
			}
		}
		else
		{
			std::stringstream ss;
			ss << obj->mNetId;
			setError("unregisterObject: Id " + ss .str() + " not valid");
		}

		obj->mNetId = InvalidNetId;
	}
}

void NetworkManager::registerStaticObject(std::uint32_t index, NetObject* obj)
{
	if (index >= mStaticNetObjects.size())
		mStaticNetObjects.resize(index + 1);
	mStaticNetObjects[index] = obj;
	if (obj)
		obj->_notifyNetwork(this);
}

void NetworkManager::setStaticObject(std::uint32_t index, NetObject* obj)
{
	if (index < mStaticNetObjects.size())
	{
		mStaticNetObjects[index] = obj;
		if (obj)
			obj->_notifyNetwork(this);
	}
}

void NetworkManager::flushDestroyObjects()
{
	if (isServer())
	{
		size_t numObjects = mNetDestroyObjects.size();
		if (numObjects > 0)
		{
			NetMessage msg(NetMessage::MSG_DestroyObject, ServerId);
			msg.writeUInt(numObjects);
			for (size_t i = 0; i < numObjects; ++i)
			{
				const DestroyObject& obj = mNetDestroyObjects[i];

				msg.writeUInt(obj.typeId);
				msg.writeUInt(obj.netId);
			}
			send(msg, ENET_PACKET_FLAG_RELIABLE);

			mNetDestroyObjects.clear();
		}
	}
}

NetObject* NetworkManager::getObject(std::uint32_t netId)
{
	if (isNetIdValid(netId))
	{
		return mNetObjects[netId];
	}
	return nullptr;
}

void NetworkManager::send(NetMessage& msg, _ENetPacketFlag packetFlag, int channel)
{
	ENetPacket* packet = enet_packet_create((char*)msg.getData(), msg.getDataSize(),
		packetFlag);

	if (channel != 2)
	{
		channel = 0;
		if (packetFlag & ENET_PACKET_FLAG_RELIABLE) channel = 0;
		else if (packetFlag & ENET_PACKET_FLAG_UNSEQUENCED) channel = 1;
	}

	if (isServer())
		enet_host_broadcast(mHost, channel, packet);
	else if (mPeer)
		enet_peer_send(mPeer, channel, packet);
}

void NetworkManager::sendTo(const std::uint16_t clientId, NetMessage& msg, _ENetPacketFlag packetFlag, int channel)
{
	ENetPacket* packet = enet_packet_create((char*)msg.getData(), msg.getDataSize(),
		packetFlag);

	if (channel != 2)
	{
		channel = 0;
		if (packetFlag & ENET_PACKET_FLAG_RELIABLE) channel = 0;
		else if (packetFlag & ENET_PACKET_FLAG_UNSEQUENCED) channel = 1;
	}

	for (size_t i = 0; i < mHost->peerCount; ++i)
	{
		if (mHost->peers[i].incomingPeerID == clientId)
		{
			enet_peer_send(&mHost->peers[i], channel, packet);
			return;
		}
	}
}

void NetworkManager::sendNotTo(const std::uint16_t clientId, NetMessage& msg, _ENetPacketFlag packetFlag, int channel)
{
	for (size_t i = 0; i < mHost->peerCount; ++i)
	{
		const std::uint16_t id = mHost->peers[i].incomingPeerID;
		if (clientId != id)
			sendTo(mHost->peers[i].incomingPeerID, msg, packetFlag, channel);
	}
}

void NetworkManager::sendServerInfo(std::uint16_t clientId)
{
	if (!isServer())
		return;

	NetMessage msg(NetMessage::MSG_ServerInfo);

	std::uint8_t numClients = getNumClients();

	msg.writeUByte(numClients);

	sendTo(clientId, msg, ENET_PACKET_FLAG_RELIABLE);
}

void NetworkManager::sendClientInfo()
{
	NetMessage msg(NetMessage::MSG_ClientInfo);
	msg.writeString(mPlayerName);

	send(msg, ENET_PACKET_FLAG_RELIABLE);
}

void NetworkManager::sendMap(const std::string& name)
{
	if (isServer())
	{
		NetMessage msg(NetMessage::MSG_MapChange);
		msg.writeUInt(mCurrSnapshotSeq);
		msg.writeString(name);
		send(msg, ENET_PACKET_FLAG_RELIABLE);
	}
	else
	{
		setError("sendMap: Not allowed send to the server");
	}
}

void NetworkManager::sendMapLoadFinished()
{
	if (!isServer())
	{
		NetMessage msg(NetMessage::MSG_MapChange);
		msg.writeUInt(mCurrSnapshotSeq);
		send(msg, ENET_PACKET_FLAG_RELIABLE);
	}
}

void NetworkManager::sendAllObjects(std::uint16_t clientId)
{
	if (isServer())
	{
		if (mNetObjects.size() > 0)
		{
			std::uint16_t numNetObjects = 0;

			NetMessage msg2;

			auto it = mNetObjects.begin();
			auto end = mNetObjects.end();
			while (it != end)
			{
				if ((*it))
				{
					NetMessage msg;
					msg.mIsReading = false;
					msg.mMsgId = NetMessage::MSG_CreateObject;
					msg.writeUInt((*it)->mTypeId);
					msg.writeUInt((*it)->mNetId);
					(*it)->writeNetCreateInit(msg);
					if ((*it)->serialize(msg))
					{
						msg2.write(msg.getData(), msg.getDataSize());
						numNetObjects++;
					}
				}
				++it;
			}

			if (numNetObjects > 0)
			{
				NetMessage sendMsg(NetMessage::MSG_CreateObject);
				sendMsg.writeUShort(numNetObjects);
				sendMsg.write(msg2.getData(), msg2.getDataSize());
				sendTo(clientId, sendMsg, ENET_PACKET_FLAG_RELIABLE);
			}
		}
	}
}

void NetworkManager::sendSpectating(std::uint32_t clientId, std::uint32_t spectator)
{
	NetMessage msg(NetMessage::MSG_Spectate);
	msg.writeUInt(spectator);
	sendTo(clientId, msg, ENET_PACKET_FLAG_RELIABLE);
}

void NetworkManager::sendSpectateNext(std::uint16_t clientId, std::uint32_t spectator)
{
	NetMessage msg(NetMessage::MSG_SpectateNext);
	msg.writeUInt(spectator);
	if (isServer())
		sendTo(clientId, msg, ENET_PACKET_FLAG_RELIABLE);
	else
		send(msg, ENET_PACKET_FLAG_RELIABLE);
}

void NetworkManager::sendUserScore()
{
	if (isServer())
	{
		NetMessage msg(NetMessage::MSG_UserScore);

		std::uint8_t size = 0;
		for (size_t i = 0; i < mLobby->mLobbyUsers.size(); ++i)
		{
			if (mLobby->mLobbyUsers[i].isConnected)
				size++;
		}

		msg.writeUByte(size);
		for (int i = 0; i < size; ++i)
		{
			if (mLobby->mLobbyUsers[i].isConnected)
			{
				msg.writeUShort(mLobby->mLobbyUsers[i].clientId);
				msg.writeUInt(mLobby->mLobbyUsers[i].score);
			}
		}

		send(msg, ENET_PACKET_FLAG_RELIABLE);
	}
}

template <class T>
void packSnapItem(T* obj, std::uint32_t snapItemId, std::vector<std::unique_ptr<NetworkManager::Connection>>& connections)
{
	VariableDeltaManager variableDelta;

	NetMessage msg;
	msg.mIsReading = false;
	msg.mMsgId = NetMessage::MSG_Sync;
	msg.mOnWriteData = std::bind(&VariableDeltaManager::onWriteData, &variableDelta, std::placeholders::_1, std::placeholders::_2);

	if (obj->serialize(msg))
	{
		variableDelta.mBasedMessage = nullptr;

		NetMessage newMsg;
		newMsg.mIsReading = false;
		newMsg.mMsgId = NetMessage::MSG_Sync;

		if (msg.getDataSize() > 0)
		{
			newMsg.write(msg.getData(), msg.getDataSize());

			for (size_t i = 0; i < connections.size(); ++i)
			{
				if (connections[i] && connections[i]->isConnected && connections[i]->isReady && connections[i]->isMapLoadFinished)
				{
					auto pIt = connections[i]->netObjectPriority.find(snapItemId);
					if (pIt == connections[i]->netObjectPriority.end())
					{
						pIt = connections[i]->netObjectPriority.insert(std::make_pair(snapItemId, 0)).first;
					}

					pIt->second += obj->calcNetPriority(connections[i]->getClientId());

					if (pIt->second >= NET_MAX_PRIORITY)
					{
						//pIt->second = 0;
						connections[i]->netObjectPriority.erase(pIt);

						connections[i]->pendingSnapShot.addItem(snapItemId, newMsg)
							.variableDelta = variableDelta;
					}
					else
					{
						SnapshotItem* item = connections[i]->snapshotManager.mBasedSnapshot.findItem(snapItemId);
						if (item)
						{
							connections[i]->pendingSnapShot.addItem(snapItemId, *item);
						}
					}
				}
			}
		}
	}
}

void NetworkManager::sendSnapShots()
{
	if (mSyncTime < mSyncFrequency)
		return;

	mSyncTime = 0.f;

	for (size_t i = 0; i < mConnections.size(); ++i)
	{
		if (mConnections[i])
		{
			mConnections[i]->pendingSnapShot.clear();
			mConnections[i]->pendingSnapShot.sequence = mCurrSnapshotSeq;
			mConnections[i]->pendingSnapShot.mTime = mConnections[i]->gameTime;
		}
	}

	for (size_t i = 0; i < mStaticNetObjects.size(); i++)
	{
		if (mStaticNetObjects[i])
			packSnapItem(mStaticNetObjects[i], i, mConnections);
	}

	const std::uint32_t snapObjectStartId = mStaticNetObjects.size();

	// add objects to Snapshot
	auto it = mNetObjects.begin();
	auto end = mNetObjects.end();
	while (it != end)
	{
		if ((*it) && (*it)->mNetworkSync)
		{
			packSnapItem((*it), (*it)->mNetId + snapObjectStartId, mConnections);
		}
		++it;
	}

	// write delta compression
	for (size_t i = 0; i < mConnections.size(); ++i)
	{
		if (mConnections[i] && mConnections[i]->isConnected && mConnections[i]->isReady && mConnections[i]->isMapLoadFinished)
		{
			NetMessage msg(NetMessage::MSG_Snap);
			msg.writeUInt(mCurrSnapshotSeq);
			msg.writeUInt(mConnections[i]->snapshotManager.mBasedSnapshot.sequence);

			mConnections[i]->pendingSnapShot.mSendFullItem = mSendFullSnapShot;
			mConnections[i]->pendingSnapShot.writeDelta(mConnections[i]->snapshotManager.mBasedSnapshot, msg);
			mConnections[i]->snapshotManager.add(mConnections[i]->pendingSnapShot);

			sendTo(mConnections[i]->getClientId(), msg, ENET_PACKET_FLAG_UNSEQUENCED);
		}
	}

	mCurrSnapshotSeq++;
}

void NetworkManager::sendClientSnapShots()
{
	if (mSyncTime >= mSyncFrequency)
	{
		mSyncTime = 0.f;

		if (mLocalPlayer)
		{
			SnapShot currSnap;
			currSnap.sequence = mCurrSnapshotSeq;
			currSnap.mTime = mInterpolationTime;

			VariableDeltaManager variableDelta;

			NetMessage currSnapMsg;
			currSnapMsg.mIsReading = false;
			currSnapMsg.mMsgId = NetMessage::MSG_Sync;
			currSnapMsg.mClientId = mClientId;
			currSnapMsg.mOnWriteData = std::bind(&VariableDeltaManager::onWriteData, &variableDelta, std::placeholders::_1, std::placeholders::_2);
			if (mLocalPlayer->serialize(currSnapMsg))
			{
				variableDelta.mBasedMessage = nullptr;

				currSnap.addItem(mLocalPlayer->mNetId, currSnapMsg)
					.variableDelta = variableDelta;

				NetMessage msg(NetMessage::MSG_Snap);
				msg.writeUInt(mCurrSnapshotSeq);

				currSnap.mSendFullItem = mSendFullSnapShot;
				currSnap.writeDelta(mSnapshotManager.mBasedSnapshot, msg);
				mSnapshotManager.add(currSnap);

				send(msg, ENET_PACKET_FLAG_UNSEQUENCED);
			}

			mCurrSnapshotSeq++;
		}
	}
}

std::uint32_t NetworkManager::getClientLastReceiveTime(const std::uint16_t clientId)
{
	for (size_t i = 0; i < mHost->peerCount; ++i)
	{
		if (mHost->peers[i].incomingPeerID == clientId)
			return mHost->peers[i].lastReceiveTime;
	}

	return 0;
}

bool NetworkManager::isClientLagged(const std::uint16_t clientId)
{
	if (mHost)
	{
		return ((mHost->serviceTime - getClientLastReceiveTime(clientId)) > 2000);
	}

	return false;
}

bool NetworkManager::isLagged()
{
	if (mPeer)
	{
		return ((mHost->serviceTime - mPeer->lastReceiveTime) > 2000);
	}

	return false;
}

bool NetworkManager::checkPlayerName(Connection* client)
{
	bool nameChanged = false;
	int playerNum = 0;
	do
	{
		nameChanged = false;
		std::string tag;
		if (playerNum > 0)
		{
			std::stringstream ss;
			ss << playerNum;
			tag = "(" + ss.str() + ")";
		}
		if (mPlayerName == client->name + tag)
		{
			playerNum++;
			nameChanged = true;
		}
		else
		{
			for (size_t i = 0; i < mConnections.size(); ++i)
			{
				if (mConnections[i] && mConnections[i]->getClientId() != client->getClientId() && mConnections[i]->isConnected)
				{
					if (mConnections[i]->name == client->name + tag)
					{
						playerNum++;
						nameChanged = true;
						break;
					}
				}
			}
		}

		if (playerNum > 0 && !nameChanged)
		{
			client->name += tag;
		}
	} while (nameChanged);

	return nameChanged;
}

int NetworkManager::calcSnapshotTotalTime()
{
	size_t prevIndex = (mSnapShotTimeHead - 1) % mSnapTimeHistory.size();

	std::int32_t totalTime = mSnapRate - mSnapCurrentTime;
	std::uint32_t lastTime = mSnapTimeHistory[prevIndex];

	size_t index = mSnapShotTimeHead;
	while (index != mSnapShotTimeTail)
	{
		std::uint32_t time = mSnapTimeHistory[index];
		totalTime += time - lastTime;
		lastTime = time;

		index = (index + 1) % mSnapTimeHistory.size();
	}
	return totalTime;
}

void NetworkManager::update(float deltaTime)
{
	std::uint32_t lastTime = mTime;
	mTime = enet_time_get();

	std::uint32_t frametime = mTime - lastTime;

	if (mIsGameRunning)
	{
		for (size_t i = 0; i < mConnections.size(); ++i)
		{
			if (mConnections[i] && mConnections[i]->isConnected && mConnections[i]->isReady)
			{
				mConnections[i]->gameTime += frametime;;
			}
		}
	}

	for (auto service : mServices)
	{
		service->update(deltaTime);
	}

	if (!mHost)
		return;

	if (mSearchLanHost)
		mSearchLanHost->update(deltaTime);

	if (mMasterClient)
		mMasterClient->update(deltaTime);

	for (auto& service : mServices)
	{
		service->update(deltaTime);
	}

	if (isServer())
		flushDestroyObjects();

	ENetEvent event;
	while (enet_host_service(mHost, &event, 0) > 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_NONE:
		{
			break;
		}
		case ENET_EVENT_TYPE_CONNECT:
		{
			std::uint16_t clientId = event.peer->incomingPeerID;

			std::uint32_t numClients = getNumClients();
			if (numClients > mMaxPlayers)
			{
				disconnectClient(clientId);
				break;
			}

			if (clientId >= mConnections.size())
			{
				mConnections.resize(clientId + 1);
			}

			mConnections[clientId] = std::make_unique<Connection>();
			Connection* client = mConnections[clientId].get();

			client->reset();
			
			client->peer = event.peer;

			char hn[260];
			if (enet_address_get_host_ip(&event.peer->address, hn, sizeof(hn)) == 0)
				client->hostName = hn;
			else
				client->hostName = "unknown";

			client->peer->data = client;
			client->isConnected = true;

			NetMessage acceptMsg(NetMessage::MSG_ConnectAccept);
			sendTo(clientId, acceptMsg);

			NetMessage connectMsg(NetMessage::MSG_Connect);
			connectMsg.writeUShort(clientId);
			sendNotTo(clientId, connectMsg);

			break;
		}
		case ENET_EVENT_TYPE_DISCONNECT:
		{
			if (isServer())
			{
				std::uint16_t clientId = event.peer->incomingPeerID;

				Connection* client = static_cast<Connection*>(event.peer->data);//getConnection(clientId);
				if (client)
				{
					client->isConnected = false;

					if (client->controller.player)
					{
						client->controller.player->onNetDestroy();
						client->controller.player = nullptr;
					}

					client->reset();
				}

				mConnections[clientId] = nullptr;

				mOnDisconnect(clientId);
			}
			else
			{
				std::uint16_t clientId = event.peer->outgoingPeerID;

				mOnDisconnect(clientId);

				disconnect();
				return;
			}
			break;
		}
		case ENET_EVENT_TYPE_RECEIVE:
		{
			NetMessage msg((char*)event.packet->data, event.packet->dataLength);
			msg.mClientId = (std::max)(event.peer->incomingPeerID, event.peer->outgoingPeerID);

			handleMessage(msg);

			for (auto service : mServices)
			{
				service->onMessage(msg);
			}
			mOnMessage(msg);

			if (event.packet->referenceCount == 0)
				enet_packet_destroy(event.packet);
			break;
		}
		}
	}

	mSyncTime += deltaTime;
	if (isServer())
	{
		sendSnapShots();
	}
	else
	{
		if (mIsGameRunning && mPrevSnapServerTime > 0)
		{
			while (mSnapCurrentTime >= mSnapRate && mSnapShotTimeHead != mSnapShotTimeTail)
			{
				mSnapCurrentTime -= mSnapRate;
				computeSnapShotTime();
			}

			mSnapCurrentTime = std::max<std::uint32_t>(0, std::min<std::uint32_t>(mSnapRate, mSnapCurrentTime));
			float lerp = (float)mSnapCurrentTime / (float)mSnapRate;
			mDebugLerp = lerp;
			mInterpolationTime = (std::uint32_t)((float)mPrevSnapServerTime + (((float)mCurrSnapServerTime - (float)mPrevSnapServerTime) * lerp));

			int snapshotTotalTime = calcSnapshotTotalTime();
			mSnapshotTotalTime = snapshotTotalTime;

			float snapshotRateScale = 1.0f;
			if (snapshotTotalTime <= 90)
			{
				snapshotRateScale = 0.95f;
			}
			else if (snapshotTotalTime > 210)
			{
				snapshotRateScale = 1.3f;
			}

			mSnapCurrentTime += std::round(frametime * snapshotRateScale);
		}

		sendClientSnapShots();
	}

	enet_host_flush(mHost);

	if (mHost->serviceTime >= mLastStatsTime + 1000)
	{
		double rate = (mHost->serviceTime - mLastStatsTime) * 0.001;
		mLastStatsTime = mHost->serviceTime;

		mTotalSendData = (mHost->totalSentData * rate) / 1024.f;
		mTotalReceivedData = (mHost->totalReceivedData * rate) / 1024.f;

		mHost->totalSentData = mHost->totalReceivedData = 0;
	}
}

void NetworkManager::handleMessage(NetMessage& msg)
{
	const std::uint8_t msgId = msg.mMsgId;
	const std::uint16_t clientId = msg.mClientId;

	switch (msgId)
	{
	case NetMessage::MSG_Snap:
		{
			handleSnapShot(msg, msgId, clientId);
			break;
		}
	case NetMessage::MSG_AckSnap:
		{
			handleAckSnapShot(msg, msgId, clientId);
			break;
		}
	case NetMessage::MSG_ObjectEvent:
	case NetMessage::MSG_ObjectEvent2:
		{
			handleObjectEvent(msg, msgId, clientId);
			break;
		}
	case NetMessage::MSG_CreateObject:
		{
			handleCreateObject(msg, msgId, clientId);
			break;
		}
	case NetMessage::MSG_DestroyObject:
		{
			handleDestroyObject(msg, msgId, clientId);
			break;
		}
	case NetMessage::MSG_ConnectAccept:
		{
			sendClientInfo();
			break;
		}
	case NetMessage::MSG_ServerInfo:
		{
			break;
		}
	case NetMessage::MSG_ClientInfo:
		{
			handleClientInfo(msg, msgId, clientId);
			break;
		}
	case NetMessage::MSG_MapChange:
		{
			handleMapChange(msg, msgId, clientId);
			break;
		}
	case NetMessage::MSG_UserScore:
		{
			std::uint8_t size = 0;
			std::uint16_t clientId = InvalidClientId;
			std::uint32_t score = 0;

			msg.readUByte(size);
			for (int i = 0; i < size; ++i)
			{
				msg.readUShort(clientId);
				msg.readUInt(score);

				Lobby::LobbyUser* user = mLobby->getUserByClientId(clientId);
				if (user)
				{
					user->score = score;
				}
			}
			break;
		}
	case NetMessage::MSG_NameChanged:
		{
			std::uint16_t id = InvalidClientId;
			std::string playerName;

			msg.readUShort(id);
			msg.readString(playerName);

			if (id == mClientId)
			{
				mPlayerName = playerName;
			}
			break;
		}
	}
}

void NetworkManager::handleSnapShot(NetMessage& msg, const std::uint8_t msgId, const std::uint16_t clientId)
{
	std::uint32_t snapSeq = 0;
	std::uint32_t basedSnapId = 0;
	msg.readUInt(snapSeq);

	if (isServer())
	{
		Connection* client = getConnection(clientId);
		if (client && client->controller.player)
		{
			if (snapSeq > client->lastSnapShot.sequence)
			{
				std::uint32_t prevTime = client->lastSnapShot.mTime;

				client->lastSnapShot.readDelta(msg);

				std::uint32_t currTime = client->lastSnapShot.mTime;

				if (currTime < client->gameTime)
				{
					client->lastSnapShot.sequence = snapSeq;
					client->snapRate = currTime - prevTime;
					client->lastSnapTime = currTime;

					mSnapServerTime = currTime;

					for (size_t i = 0; i < client->lastSnapShot.getNumItems(); ++i)
					{
						client->lastSnapShot.mItems[i].msg.mMsgId = NetMessage::MSG_Sync;
						client->lastSnapShot.mItems[i].msg.mClientId = clientId;
						client->lastSnapShot.mItems[i].msg.mROffset = 0;

						std::uint32_t snapItemId = client->lastSnapShot.mItems[i].id;

						VariableDeltaManager variableDelta = client->lastSnapShot.mItems[i].variableDelta;
						variableDelta.setBasedMsg(&client->lastSnapShot.mItems[i].msg);

						NetObject* obj = getObject(snapItemId);
						if (obj)
						{
							obj->serialize(client->lastSnapShot.mItems[i].msg);
						}
					}

					NetMessage ackMsg(NetMessage::MSG_AckSnap);
					ackMsg.writeUInt(snapSeq);
					sendTo(clientId, ackMsg, ENET_PACKET_FLAG_UNSEQUENCED);
				}
			}
		}
	}
	else
	{
		const std::uint32_t snapObjectStartId = mStaticNetObjects.size();

		// Client side
		if (snapSeq > mSnapShot.sequence)
		{
			msg.readUInt(basedSnapId);

			std::uint32_t prevTime = mSnapShot.mTime;

			mSnapShot.sequence = snapSeq;
			mSnapShot.readDelta(msg);

			// server time
			std::uint32_t currTime = mSnapShot.mTime;
			mSnapServerTime = currTime;

			if (mSnapShotTimeHead == mSnapShotTimeTail && mSnapShotTimeTail != 0)
			{
				computeSnapShotTime();
			}

			mSnapTimeHistory[mSnapShotTimeTail] = currTime;
			mSnapShotTimeTail = (mSnapShotTimeTail + 1) % mSnapTimeHistory.size();

			if (mPrevSnapServerTime == 0)
			{
				computeSnapShotTime();
			}

			for (size_t i = 0; i < mSnapShot.getNumItems(); ++i)
			{
				mSnapShot.mItems[i].msg.mMsgId = NetMessage::MSG_Sync;
				mSnapShot.mItems[i].msg.mROffset = 0;

				std::uint32_t snapItemId = mSnapShot.mItems[i].id;

				VariableDeltaManager variableDelta = mSnapShot.mItems[i].variableDelta;
				variableDelta.setBasedMsg(&mSnapShot.mItems[i].msg);

				if (snapItemId < snapObjectStartId)
				{
					if (snapItemId < mStaticNetObjects.size() && mStaticNetObjects[snapItemId])
					{
						mStaticNetObjects[i]->serialize(mSnapShot.mItems[i].msg);
					}
				}
				else
				{
					std::uint32_t netId = snapItemId - mStaticNetObjects.size();

					NetObject* obj = getObject(netId);
					if (obj)
					{
						obj->serialize(mSnapShot.mItems[i].msg);
					}
				}
			}

			NetMessage ackMsg(NetMessage::MSG_AckSnap);
			ackMsg.writeUInt(snapSeq);
			sendTo(clientId, ackMsg, ENET_PACKET_FLAG_UNSEQUENCED);
		}
	}
}

void NetworkManager::handleAckSnapShot(NetMessage& msg, const std::uint8_t msgId, const std::uint16_t clientId)
{
	std::uint32_t snapId = 0;

	msg.readUInt(snapId);

	if (isServer())
	{
		Connection* conn = getConnection(clientId);
		if (conn)
		{
			conn->snapshotManager.ackSnapshot(snapId);
		}
	}
	else
	{
		mSnapshotManager.ackSnapshot(snapId);
	}
}

void NetworkManager::handleCreateObject(NetMessage& msg, const std::uint8_t msgId, const std::uint16_t clientId)
{
	std::uint16_t numObjects = 0;
	std::uint32_t typeId;
	std::uint32_t netId;

	msg.readUShort(numObjects);

	for (std::uint16_t i = 0; i < numObjects; ++i)
	{
		msg.readUInt(typeId);
		msg.readUInt(netId);

		if (!getObject(netId))
		{
			NetObject* obj = mOnHandleCreateObject(typeId, msg);
			if (obj)
			{
				registerObject(obj, netId);
				obj->serialize(msg);
			}
			else
			{
				std::stringstream ss;
				ss << typeId;
				setError("MSG_CreateObject: typeId " + ss.str() + " not found");

				break;
			}
		}
		else
		{
			std::stringstream ss;
			ss << typeId;
			setError("MSG_CreateObject: netId " + ss.str() + " already created");

			break;
		}
	}
}

void NetworkManager::handleDestroyObject(NetMessage& msg, const std::uint8_t msgId, const std::uint16_t clientId)
{
	std::uint32_t numObjects;
	std::uint32_t typeId;
	std::uint32_t netId;

	msg.readUInt(numObjects);
	for (std::uint32_t i = 0; i < numObjects; ++i)
	{
		msg.readUInt(typeId);
		msg.readUInt(netId);

		NetObject* obj = getObject(netId);
		if (obj)
		{
			obj->serialize(msg);
			obj->onNetDestroy();
			unregisterObject(obj);
		}
		else
		{
			std::stringstream ss;
			ss << netId;
			setError("MSG_DestroyObject: netId " + ss.str() + " not found");
		}
	}
}

void NetworkManager::handleObjectEvent(NetMessage& msg, const std::uint8_t msgId, const std::uint16_t clientId)
{
	std::uint32_t netId = InvalidNetId;
	std::uint8_t eventId = 0;
	std::uint32_t time = 0;

	msg.readUInt(netId);
	msg.readUByte(eventId);
	msg.readUInt(time);

	NetObject* obj = getObject(netId);
	if (!obj)
	{
		std::stringstream ss;
		ss << netId;
		setError("handleObjectEvent: netId not found " + ss.str());
		return;
	}

	if (msg.mMsgId == NetMessage::MSG_ObjectEvent)
	{
		obj->onNetEvent(eventId);
	}
	else
	{
		std::uint16_t dataSize = 0;
		msg.readUShort(dataSize);

		NetMessage newMsg;
		newMsg.mBuffer.resize(dataSize);
		msg.read(newMsg.getData(), dataSize);

		obj->onNetEvent(eventId, newMsg);
	}
}

void NetworkManager::handleClientInfo(NetMessage& msg, const std::uint8_t msgId, const std::uint16_t clientId)
{
	Connection* client = getConnection(clientId);
	if (client)
	{
		if (!client->isReady)
		{
			client->isReady = true;
			client->gameTime = 0;

			msg.readString(client->name);

			if (checkPlayerName(client))
			{
				NetMessage changedMsg(NetMessage::MSG_NameChanged);
				changedMsg.writeUShort(clientId);
				changedMsg.writeString(client->name);
				sendTo(clientId, changedMsg);
			}

			mOnConnect(clientId);

			sendServerInfo(clientId);

			if (mIsGameRunning)
			{
				//if (client->isMapLoadFinished)
				{
					NetMessage newMsg(NetMessage::MSG_MapChange);
					newMsg.writeUInt(mCurrSnapshotSeq);
					mOnWriteMapInfo(newMsg);
					sendTo(clientId, newMsg, ENET_PACKET_FLAG_RELIABLE);
				}
			}
		}
	}
}

void NetworkManager::handleMapChange(NetMessage& msg, const std::uint8_t msgId, const std::uint16_t clientId)
{
	if (isServer())
	{
		std::uint32_t sequence = 0;
		msg.readUInt(sequence);

		Connection* client = getConnection(clientId);
		if (client)
		{
			client->isMapLoadFinished = true;
			client->gameTime = 0;
			client->lastSnapShot.sequence = sequence;
		}

		sendAllObjects(clientId);

		bool allReady = true;
		for (size_t i = 0; i < mConnections.size(); ++i)
		{
			if (mConnections[i] && mConnections[i]->isConnected)
			{
				if (!mConnections[i]->isMapLoadFinished)
				{
					allReady = false;
					break;
				}
			}
		}

		if (allReady)
		{
			mOnAllClientsLoadingFinished();
		}
	}
}

void NetworkManager::computeSnapShotTime()
{
	if (mSnapShotTimeTail == mSnapShotTimeHead)
	{
		return;
	}

	std::uint32_t snapTime = mSnapTimeHistory[mSnapShotTimeHead];
	std::uint32_t time = enet_time_get();

	mPrevSnapServerTime = mCurrSnapServerTime;
	mCurrSnapServerTime = snapTime;
	mSnapRate = mCurrSnapServerTime - mPrevSnapServerTime;
	if (mSnapRate < 0)
		mSnapRate = 100;

	mSnapShotTimeHead = (mSnapShotTimeHead + 1) % mSnapTimeHistory.size();
}

void NetworkManager::onWriteServerInfo(NetMessage& msg)
{
	mOnWriteServerInfo(msg);
}