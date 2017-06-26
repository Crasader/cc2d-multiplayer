#pragma once

#include "MenuBase.h"

#include "ui\UIWidget.h"
#include "ui\CocosGUI.h"

#include <enet/enet.h>

class MasterClient;

class ServerBrowser : public MenuBase
{
public:

	enum SearchType
	{
		ST_INTERNET,
		ST_LAN
	};
	struct ServerInfo
	{
		enum StatusFlags
		{
			STATUS_NEW = 0,
			STATUS_DEDICATED = (1 << 0),
			STATUS_PASSWORDED = (1 << 1),
			STATUS_QUERYING = (1 << 2),
			STATUS_UPDATING = (1 << 3),
			STATUS_RESPONEDED = (1 << 4),
			STATUS_TIMEDOUT = (1 << 5)
		};

		ENetAddress address;
		enet_uint16 infoPort;

		enet_uint32 status = STATUS_NEW;

		std::string name;
		std::string mapName;
		std::string modeName;
		std::string desc;
		enet_uint32 numPlayers;
		enet_uint32 maxPlayers;
		enet_uint32 version;
		enet_uint32 latency = 999;
		enet_uint32 requestTime = 0;
		bool password = false;
		bool isFavorite;
		bool inLobby = true;

		SearchType searchType = ST_INTERNET;
		bool isFiltered = false;

		int tabIndex = -1;
		cocos2d::RefPtr<cocos2d::ui::Layout> layout = nullptr;
		cocos2d::Ref* joinButton = nullptr;
		cocos2d::ui::Text* latenceText = nullptr;
		cocos2d::ui::Text* mapText = nullptr;
		cocos2d::ui::Text* modeText = nullptr;
		cocos2d::ui::Text* numPlayerText = nullptr;
		cocos2d::ui::Text* maxPlayerText = nullptr;
	};

	struct FilterServer
	{
		std::string mapName;
		std::string modeName;
		bool isNotFull = false;
		bool hasPlayers = false;
		bool noPassword = false;
		std::uint32_t maxLatency = 0;
	};

public:

	static cocos2d::Scene* createScene(GameSystem* game);
	bool init();
	virtual void init(GameSystem* game);

	void createServerLayout(ServerInfo* serverInfo);
	void updateServerLayout(ServerInfo* serverInfo);

	void onRefreshServerList(cocos2d::Ref* sender);
	void onJoinServer(cocos2d::Ref* sender);
	void onConnectToServer(cocos2d::Ref* sender);
	void onSelectedItemEvent(Ref* sender, cocos2d::ui::ListView::EventType type);

	void onBack(cocos2d::Ref* sender);

	bool updateServerListFromMaster();

	void onServerListReady(MasterClient* masterClient);

	bool pingServers();

	void onLanServerFound(std::uint32_t host, std::uint16_t port);

	void filterServers();
	void sortServers();

	void update(float deltaTime);

	virtual void onEnter() override;
	virtual void onExit() override;

	ENetSocket mServerInfoSocket = ENET_SOCKET_NULL;

	std::uint32_t mMaxPingServers = 5;
	std::uint32_t mLastPingServer = 0;
	enet_uint32 mServerPingRate = 10000;
	enet_uint32 mLastInfo = 0;
	std::vector<ServerInfo> mServerInfos;

	cocos2d::ui::TabControl* mTabControl = nullptr;
	cocos2d::ui::ListView* mInternetListView = nullptr;
	cocos2d::ui::ListView* mLanListView = nullptr;

	cocos2d::ui::TextField* mServerAdressTextField = nullptr;

	FilterServer mFilterServer;

	CREATE_FUNC(ServerBrowser);
};