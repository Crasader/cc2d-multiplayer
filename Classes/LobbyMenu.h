#ifndef __LOBBYMENU_H__
#define __LOBBYMENU_H__

#include "cocos2d.h"

#include "ui\CocosGUI.h"

#include "MenuBase.h"

class GameSystem;
class Lobby;
class NetMessage;

class LobbyMenu : public MenuBase
{
public:
	LobbyMenu();
	~LobbyMenu();

	static cocos2d::Scene* createScene(GameSystem* game);
	virtual bool init();
	void initMenu();

	void addUser(std::uint16_t clientId, const std::string name);
	void addUser(size_t index, std::uint16_t clientId, const std::string name);
	void removeUser(std::uint16_t clientId);
	void removeUserAtIndex(size_t index);

	void changeMap(cocos2d::Ref* sender);
	void setMapName(const std::string& map);

	void onMinus(cocos2d::Ref* sender);
	void onPlus(cocos2d::Ref* sender);

	void onStartGame(cocos2d::Ref* sender);

	void onSendChat(cocos2d::Ref* sender);
	void addChatText(std::uint16_t clientId, const std::string& text);

	void onBack(cocos2d::Ref* sender);

	void update(float deltaTime);

	virtual void onEnter() override;
	virtual void onExit() override;

	void onClientConnect(std::uint32_t clientId);
	void onClientDisconnect(std::uint32_t clientId);
	void onNetMessage(NetMessage& msg);

	void sendChangeMap();

	CREATE_FUNC(LobbyMenu);

private:
	cocos2d::ui::Text* mMapText = nullptr;

	cocos2d::ui::Text* mRoundsNumLabel = nullptr;

	cocos2d::ui::ScrollView* mChatScrollView;
	cocos2d::ui::Text* mChatHistory = nullptr;
	cocos2d::ui::TextField* mChatTextField = nullptr;

	std::vector<cocos2d::ui::Button*> mUserSlots;

	Lobby* mLobby;
};

#endif