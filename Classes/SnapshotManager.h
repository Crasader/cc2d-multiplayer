#ifndef __SNAPSHOTMANAGER_H__
#define __SNAPSHOTMANAGER_H__

#include <list>

#include "SnapShot.h"

class SnapshotManager
{
public:
	void reset();

	void add(SnapShot& snapshot);

	void removeOlderThan(std::uint32_t seq);

	void ackSnapshot(std::uint32_t seq);

	SnapShot mBasedSnapshot;

	std::list<SnapShot> mSnapshotList;
};

#endif