#ifndef __SNAPSHOT_H__
#define __SNAPSHOT_H__

#include <vector>

#include "NetMessage.h"
#include "VariableDeltaManager.h"

struct SnapshotItem
{
	std::uint32_t id;
	NetMessage msg;
	VariableDeltaManager variableDelta;
	bool isBased;
};

class SnapShot
{
public:
	SnapShot();
	~SnapShot();

	void clear();

	size_t getNumItems() const { return mItems.size(); }
	bool getItemMsg(size_t index, NetMessage& msg);

	SnapshotItem* findItem(size_t id);

	SnapshotItem& findOrCreateItem(std::uint32_t id);

	SnapshotItem& addItem(std::uint32_t id, const NetMessage& msg);
	SnapshotItem& addItem(std::uint32_t id, SnapshotItem& item);
	void removeItem(std::uint32_t id);

	bool writeDelta(SnapShot& oldSnapshot, NetMessage& msg);
	void readDelta(NetMessage& msg);

	bool itemSame(SnapshotItem& oldState, SnapshotItem& newState);

	std::uint32_t sequence = 0;

	std::uint32_t mTime = 0;
	std::uint32_t mRecvTime = 0;

	std::vector<SnapshotItem> mItems;

	bool mSendFullItem = false;
};

#endif