#ifndef __SCRIPTGAMEMODE_H__
#define __SCRIPTGAMEMODE_H__

#include "GameMode.h"

class ScriptGameMode : public GameMode
{
public:
	ScriptGameMode(GameSystem* game, const std::string& modeName);
	virtual ~ScriptGameMode();

	virtual void clientConnected(std::uint16_t clientId) override;
	virtual void clientDisconnected(std::uint16_t clientId) override;

	virtual void onStart() override;
	virtual void onAllClientsLoadingFinished() override;

	virtual cocos2d::Vec2 getPlayerSpawnSpot(std::uint16_t clientId) override;

	virtual void onSpawnPlayer(Player* player) override;
	virtual void onCharacterDestroyed(class Character* character) override;

	virtual void update(float deltaTime) override;

	virtual bool serialize(NetMessage& msg) override;
};

#endif