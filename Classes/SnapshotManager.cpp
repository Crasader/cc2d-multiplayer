#include "SnapshotManager.h"

#include "VariableDeltaManager.h"

void SnapshotManager::reset()
{
	mSnapshotList.clear();
	mBasedSnapshot.clear();
}

void SnapshotManager::add(SnapShot& snapshot)
{
	if (mSnapshotList.size() > 10)
	{
		if (mSnapshotList.front().sequence == mBasedSnapshot.sequence)
		{
			mBasedSnapshot.sequence = 0;
			mBasedSnapshot.clear();
		}
		mSnapshotList.pop_front();
	}

	auto it = mSnapshotList.begin();
	auto end = mSnapshotList.end();
	while (it != end)
	{
		if (snapshot.sequence < it->sequence)
		{
			mSnapshotList.insert(it, snapshot);
			return;
		}
		++it;
	}

	mSnapshotList.push_back(snapshot);
}

void SnapshotManager::removeOlderThan(std::uint32_t seq)
{
	auto it = mSnapshotList.begin();
	while (it != mSnapshotList.end())
	{
		if (it->sequence < seq)
			it = mSnapshotList.erase(it);
		else
			return;
	}
}

void SnapshotManager::ackSnapshot(std::uint32_t seq)
{
	auto it = mSnapshotList.begin();
	while (it != mSnapshotList.end())
	{
		if (it->sequence < seq)
			it = mSnapshotList.erase(it);
		else if (it->sequence == seq)
		{
			mBasedSnapshot = (*it);
			return;
		}
		else
		{
			return;
		}
	}
}