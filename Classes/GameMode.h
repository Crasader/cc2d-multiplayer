#ifndef __GAMEMODE_H__
#define __GAMEMODE_H__

#include <cstdint>
#include "math/Vec2.h"
#include "2d/CCLabel.h"

#include "NetObject.h"

#include "TransformInterpolation.h"

class GameSystem;
class GameObject;
class Player;

class NetMessage;

class GameMode : public NetObject
{
public:
	enum GameState
	{
		GS_NONE,
		GS_WARMUP,
		GS_ACTIVE
	};

	struct Score
	{
		std::int16_t score = 0;
		cocos2d::Label* scoreLabel = nullptr;
	};

	struct GameModeState
	{
		std::uint32_t time;
		std::uint32_t gameState;
		bool gameOver = false;
		bool isCountdownVisible = false;
		int countdown = 0;
	};

public:
	GameMode(GameSystem* game, const std::string& modeName);
	virtual ~GameMode();

	const std::string& getModeName() const { return mModeName; }

	void setTeam(std::uint16_t clientId, int team);
	void setTeam(Player* player, int team);

	int getTeam(std::uint16_t clientId);
	int getTeam(Player* player);

	size_t getNumClientsInTeam(int team);
	void getClientsInTeam(int team, std::vector<std::uint16_t>& clients);

	virtual void onStart() {}
	virtual void onFinishedLoading() {}
	virtual void onAllClientsLoadingFinished() {}
	virtual void onEnd() {}

	virtual void checkPlayerRespawn(std::uint32_t respawnTime = 0);

	virtual bool hasEnoughPlayers() { return true; }

	virtual bool isTeamBased() { return false; }
	virtual bool isAllowedToSpawn(GameObject* obj) { return true; }

	virtual void clientConnected(std::uint16_t clientId) {}
	virtual void clientDisconnected(std::uint16_t clientId) {}

	virtual cocos2d::Vec2 getPlayerSpawnSpot(std::uint16_t clientId) { return cocos2d::Vec2(); }
	virtual void getPlayerSpawnSpot(GameObject* obj) {}

	virtual void onSpawnPlayer(Player* player) {}
	virtual void onCharacterDestroyed(class Character* character) {}
	virtual void onDeath(Character* character) {}

	void initScore(size_t numTeams);
	void setScore(size_t index, int score);

	void showCountdown(bool show);
	void setCountdown(int time);

	virtual void update(float deltaTime);

	virtual std::uint32_t calcNetPriority(std::uint16_t clientId) override;
	virtual bool serialize(NetMessage& msg) override;

	GameSystem* mGame = nullptr;
	std::string mModeName;

	cocos2d::Label* mCountdownLabel = nullptr;

	std::vector<Score> mScores;

	StateHistory<GameModeState> mGameModeState;
	GameModeState mCurrentState;
};

#endif