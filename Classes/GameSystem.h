#ifndef __GAMESYSTEM_H__
#define __GAMESYSTEM_H__

#include <vector>

#include "math/Vec2.h"
#include "2d/CCNode.h"
#include "2d/CCAction.h"
#include "2d/CCDrawNode.h"
#include "2d/CCLabel.h"

#include "Parameters.h"

#include "Scoreboard.h"

class GameObject;

class Input;
class Physics;
class NetworkManager;
class ScriptManager;

class LevelScene;
class Lobby;
class GameMode;

class Player;
class NetObject;
class NetMessage;

class DebugDrawer;

class GameSystem
{
public:
	struct DebugLine
	{
		float lifeTime;
		cocos2d::Vec2 start;
		cocos2d::Vec2 end;
		cocos2d::Color4F color;
	};
	struct DebugText
	{
		std::string text;
		double realValue;
		double maxValue;
		double drawValue;
		float updateTime;
		float nextUpdateTime;
		bool drawMax;
	};

public:
	GameSystem();
	~GameSystem();

	void setError(const std::string& error);
	void onNetError(const std::string& error);

	void addNode(cocos2d::Node* node, int zOrder);

	Player* spawnPlayer(std::uint16_t clientId, const cocos2d::Vec2& spawnPos);
	void removePlayer(std::uint32_t clientId);

	GameObject* spawnGameObject(const std::string& className, const cocos2d::Vec2& pos, Parameters& spawnArgs);

	void addGameObject(GameObject* obj);
	void removeGameObject(GameObject* obj);
	void _removeGameObject(GameObject* obj);
	void _removeAllGameObjects();

	GameObject* getGameObject(std::uint32_t index);
	GameObject* getGameObjectByName(const std::string& name);

	template <class T>
	void findObjects(std::vector<T*>& objects)
	{
		for (size_t i = 0; i < mGameObjects.size(); ++i)
		{
			if (!mGameObjects[i]->mIsDestroyed)
			{
				T* obj = dynamic_cast<T*>(mGameObjects[i]);
				if (obj)
					objects.push_back(obj);
			}
		}
	}

	void setCameraTarget(cocos2d::Node* node);

	void setMyPlayer(Player* player);

	std::uint32_t calcPlayerNum();

	void scoreUser(std::uint16_t clientId);
	void scoreTeam(int team);
	void resetUserScores();

	void spectating(std::uint32_t clientId);
	void spectating(std::uint32_t clientId, NetObject* spectator);
	void spectateNext(std::uint32_t clientId);
	void onSpectateNext();

	void loadMapNames();

	void setNextMapName(const std::string& mapName);
	std::string getNextMap() const { return mNextMapName; }

	std::string getCurrMapName() const { return mMapName; }

	void loadMap(const std::string& mapName);
	void unloadMap();

	void changeToNextRandomMap();

	void wantsRestart();

	void startGame();
	void exitGame();

	void onExitGame();

	void onAllClientsLoadingFinished();

	void readConfig();
	void writeConfig();

	void setGameOver();
	bool isGameOver() const { return mIsGameOver; }

	void onGameFinished();

	void update(float deltaTime);

	void onStartVote();

	void onWriteMapInfo(NetMessage& msg);
	void onWriteServerInfo(NetMessage& msg);
	NetObject* onHandleCreateNetObject(std::uint32_t typeId, NetMessage& msg);
	void handleMessage(NetMessage& msg);

	void toggleDebug();

	void addDebugLine(const cocos2d::Vec2& start, const cocos2d::Vec2& end, const cocos2d::Color4F& color, float lifeTime = 0.0f);
	void addDebugRect(const cocos2d::Vec2& start, const cocos2d::Vec2& dest, const cocos2d::Color4F& color, float lifeTime = 0.0f);

	static int addDebugText(const std::string& text, float updateTime, bool drawMax = false);
	void setDebugText(int index, double value);
	void resetDebugText(int index);

	std::unique_ptr<Input> mInput;
	std::unique_ptr<Physics> mPhysics;
	std::unique_ptr<NetworkManager> mNetwork;
	std::unique_ptr<ScriptManager> mScriptManager;

	std::vector<GameObject*> mGameObjects;
	std::vector<GameObject*> mRemoveList;

	LevelScene* mLevelScene = nullptr;

	cocos2d::Follow* mCameraTarget = nullptr;

	float mLevelHeight = 0;
	float mLevelWidth = 0;

private:
	std::string mPrevMapName;
	std::string mNextMapName;
	std::string mMapName;

public:

	bool mIsGameRunning = false;
	bool mIsMapLoaded = false;
	bool mAllClientsLoadingFinished = false;

	bool mIsGameOver = false;
	float mGameOverWaitTime = 0.f;

	bool mIsRestarting = false;

	std::uint32_t mGameTick = 0;
	float mTime = 0.0f;

	std::unique_ptr<GameMode> mGameMode;
	std::unique_ptr<Scoreboard> mScoreboard;

	std::uint32_t mMaxPlayers = 4;

	int mTotalRoundCounter = 2;
	int mCurrentRoundCounter = 0;

	Lobby* mLobby = nullptr;

	Player* mMyPlayer = nullptr;

	std::vector<std::string> mMapNames;

	Parameters mConfig;

	cocos2d::Scene* mScoreScene = nullptr;

	cocos2d::Label* mGameOverLabel = nullptr;
	cocos2d::Label* mSpectatingTextLabel = nullptr;

	bool mIsDebugEnable = true;

	std::unique_ptr<DebugDrawer> mPhysicsDebugDrawer;

	cocos2d::DrawNode* mDebugDrawer = nullptr;
	std::vector<DebugLine> mDebugLines;
	cocos2d::Label* mDebugTextLabel = nullptr;
	static std::vector<DebugText> mDebugTextVec;

	class ChartPanel* mChart = nullptr;
};

#endif