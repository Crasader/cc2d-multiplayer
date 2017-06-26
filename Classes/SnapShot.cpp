#include "Snapshot.h"

#include <algorithm>

SnapShot::SnapShot()
{
}

SnapShot::~SnapShot()
{

}

void SnapShot::clear()
{
	mItems.clear();
	mTime = 0.0f;
	mRecvTime = 0.0f;
	sequence = 0;
}

bool SnapShot::getItemMsg(size_t index, NetMessage& msg)
{
	if (index >= mItems.size())
		return false;

	msg = mItems[index].msg;

	return true;
}

struct SnapshotItemCmp
{
	bool operator () (const SnapshotItem& left, std::uint32_t right) const
	{
		return left.id < right;
	}

	bool operator () (std::uint32_t left, const SnapshotItem& right) const
	{
		return left < right.id;
	}

	bool operator () (const SnapshotItem& left, const SnapshotItem& right) const
	{
		return left.id < right.id;
	}
};

SnapshotItem* SnapShot::findItem(size_t id)
{
	auto it = std::lower_bound(mItems.begin(), mItems.end(), id, SnapshotItemCmp());
	if (it != mItems.end() && it->id == id)
	{
		return &(*it);
	}

	return nullptr;
}

SnapshotItem& SnapShot::findOrCreateItem(std::uint32_t id)
{
	auto it = std::lower_bound(mItems.begin(), mItems.end(), id, SnapshotItemCmp());
	if (it != mItems.end() && it->id == id)
	{
		return (*it);
	}

	SnapshotItem item;
	item.id = id;
	it = mItems.insert(it, item);

	return (*it);
}

SnapshotItem& SnapShot::addItem(std::uint32_t id, const NetMessage& msg)
{
	SnapshotItem& item = findOrCreateItem(id);
	item.msg = msg;
	item.isBased = false;

	return item;
}

SnapshotItem& SnapShot::addItem(std::uint32_t id, SnapshotItem& cpyItem)
{
	SnapshotItem& item = findOrCreateItem(id);
	item.msg = cpyItem.msg;
	item.variableDelta = cpyItem.variableDelta;
	item.isBased = true;

	return item;
}

void SnapShot::removeItem(std::uint32_t id)
{
	auto it = std::lower_bound(mItems.begin(), mItems.end(), id, SnapshotItemCmp());
	if (it != mItems.end() && it->id == id)
	{
		it = mItems.erase(it);
	}
}

bool SnapShot::writeDelta(SnapShot& oldSnapshot, NetMessage& msg)
{
	msg.writeUInt(mTime);

	bool hasData = false;

	size_t oldIndex = 0;
	for (size_t itemIndex = 0; itemIndex < mItems.size(); ++itemIndex)
	{
		SnapshotItem& currSnapItem = mItems[itemIndex];
		uint16_t itemId = static_cast<int16_t>(currSnapItem.id);

		if (oldIndex >= oldSnapshot.mItems.size())
		{
			hasData = true;

			// new item
			msg.writeUShort(itemId);
			msg.writeUShort(currSnapItem.msg.getDataSize());

			currSnapItem.variableDelta.writeChanged(0, currSnapItem.msg, msg);
			for (size_t j = 0; j < currSnapItem.variableDelta.mVariableChanged.size(); ++j)
			{
				msg.writeUByte(currSnapItem.variableDelta.mVariableChanged[j]);
			}

			msg.write(currSnapItem.msg.getData(), currSnapItem.msg.getDataSize());
		}
		else
		{
			for (; oldIndex < oldSnapshot.mItems.size() && itemId > oldSnapshot.mItems[oldIndex].id; ++oldIndex)
			{
				hasData = true;

				// old item
				SnapshotItem& oldSnapItem = oldSnapshot.mItems[oldIndex];
				msg.writeUShort(oldSnapItem.id);
				msg.writeUShort(0);
			}

			if (oldIndex >= oldSnapshot.mItems.size())
				oldIndex = oldSnapshot.mItems.size() - 1;

			SnapshotItem& oldSnapItem = oldSnapshot.mItems[oldIndex];
			if (itemId == oldSnapItem.id)
			{
				if (!itemSame(oldSnapItem, currSnapItem) || mSendFullItem)
				{
					if (!currSnapItem.isBased)
					{
						hasData = true;

						// same item, but data changed
						msg.writeUShort(itemId);

						if (mSendFullItem)
						{
							msg.writeUShort(currSnapItem.msg.getDataSize());

							currSnapItem.variableDelta.writeChanged(0, currSnapItem.msg, msg);
							for (size_t j = 0; j < currSnapItem.variableDelta.mVariableChanged.size(); ++j)
							{
								msg.writeUByte(currSnapItem.variableDelta.mVariableChanged[j]);
							}

							msg.write(currSnapItem.msg.getData(), currSnapItem.msg.getDataSize());
						}
						else
						{
							size_t dataSize = currSnapItem.variableDelta.writeChanged(&oldSnapItem.msg, currSnapItem.msg, msg);
							msg.writeUShort(dataSize);

							for (size_t j = 0; j < currSnapItem.variableDelta.mVariableChanged.size(); ++j)
							{
								msg.writeUByte(currSnapItem.variableDelta.mVariableChanged[j]);
							}

							currSnapItem.variableDelta.writeDelta(oldSnapItem.msg, currSnapItem.msg, msg);
						}
					}
				}

				++oldIndex;
			}
			else
			{
				hasData = true;

				// new item
				msg.writeUShort(itemId);
				msg.writeUShort(currSnapItem.msg.getDataSize());

				currSnapItem.variableDelta.writeChanged(0, currSnapItem.msg, msg);
				for (size_t j = 0; j < currSnapItem.variableDelta.mVariableChanged.size(); ++j)
				{
					msg.writeUByte(currSnapItem.variableDelta.mVariableChanged[j]);
				}

				msg.write(currSnapItem.msg.getData(), currSnapItem.msg.getDataSize());
			}
		}
	}

	for (; oldIndex < oldSnapshot.mItems.size(); ++oldIndex)
	{
		hasData = true;

		// old item
		SnapshotItem& oldSnapItem = oldSnapshot.mItems[oldIndex];
		msg.writeUShort(oldSnapItem.id);
		msg.writeUShort(0);
	}

	return hasData;
}

void SnapShot::readDelta(NetMessage& msg)
{
	msg.readUInt(mTime);

	uint16_t itemId = 0;
	while (msg.readUShort(itemId))
	{
		uint16_t newSize = 0;
		msg.readUShort(newSize);
		if (newSize == 0)
		{
			removeItem(itemId);
		}
		else
		{
			VariableDeltaManager variableDelta;
			variableDelta.readChangedVariables(msg);

			NetMessage newMsg;
			newMsg.mIsReading = true;
			newMsg.mBuffer.resize(newSize);
			msg.read(newMsg.getData(), newSize);

			addItem(itemId, newMsg)
				.variableDelta = variableDelta;
		}
	}
}

bool SnapShot::itemSame(SnapshotItem& oldState, SnapshotItem& newState)
{
	if (oldState.msg.getDataSize() != newState.msg.getDataSize())
		return false;

	if (memcmp(oldState.msg.getData(), newState.msg.getData(), newState.msg.getDataSize()) == 0)
		return true;

	return false;
}