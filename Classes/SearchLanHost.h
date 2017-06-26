#ifndef __SEARCHLANHOST_H__
#define __SEARCHLANHOST_H__

#include <enet/enet.h>

#include <functional>
#include <cstdint>

#include "NetService.h"

class SearchLanHost : public NetService
{
public:
	enum eMessageType
	{
		MT_None,
		MT_Search,
		MT_SearchResult,
	};
public:
	SearchLanHost(bool isHost = false, std::uint16_t port = 0);
	~SearchLanHost();

	void startSearch(enet_uint32 timeout = 5000);
	void abortSearch();

	bool isDone() const;

	void update(float deltaTime) override;

	std::function<void(std::uint32_t, std::uint16_t)> mOnFound;

private:
	ENetSocket mSocket = ENET_SOCKET_NULL;

	bool mIsDone = false;
	bool mIsHost = false;
	std::uint16_t mHostPort = 0;

	enet_uint32 mStartTime = 0;
	enet_uint32 mLastSendTime = 0;

	enet_uint32 mSearchTimeout = 5000;
};

#endif