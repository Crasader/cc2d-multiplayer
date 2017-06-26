#ifndef __SCRIPTMANAGER_H__
#define __SCRIPTMANAGER_H__

#include "angelscript.h"

#include "Script.h"

#include "cocos2d.h"

class GameSystem;
class Parameters;
class GameObject;

class ScriptObject
{
public:
	ScriptObject(ScriptManager* mgr, GameObject* obj)
		: mScriptManager(mgr)
		, mObject(obj)
	{}

	int AddRef();
	int Release();

	std::uint32_t getLocalIndex() const;
	std::uint32_t getType() const;
	cocos2d::Vec2 getPosition();

	ScriptManager* mScriptManager = nullptr;
	GameObject* mObject = nullptr;

	int refCount = 0;
	size_t mLocalIndex = std::numeric_limits<std::uint32_t>::max();
};

class ScriptManager
{
public:
	ScriptManager(GameSystem* game);
	~ScriptManager();

	Script* createScript(const std::string& name, const std::string& file);
	bool loadFile(const std::string& file);

	asIScriptContext* prepareContextFromPool(asIScriptFunction* func);
	void returnContextToPool(asIScriptContext* ctx);

	void messageCallback(const asSMessageInfo* msg);

	ScriptObject* createScriptObject(GameObject* obj);
	void removeScriptObject(ScriptObject* obj);

	bool isServer();
	bool isClient();

	void setGameOver();
	bool isGameOver();

	void scoreUser(std::uint16_t clientId);
	void scoreTeam(int team);

	void spawnPlayer(const std::string name);
	std::uint32_t spawnObject(const std::string& name, const cocos2d::Vec2 pos, Parameters& params);
	void destroyObject(std::uint32_t index);

	GameObject* getGameObjectByName(const std::string& name);
	std::uint32_t getObjectIndexByName(const std::string& name);

	class CScriptArray* findObjectsByType(int type);
	class CScriptArray* findObjectsIndexByType(std::uint32_t type);

	std::uint16_t getClientIdFromPlayer(std::uint32_t index);

	std::uint32_t getObjectType(std::uint32_t index);
	std::string getObjectName(std::uint32_t index);
	cocos2d::Vec2 getObjectPosition(std::uint32_t index);
	void setObjectPosition(std::uint32_t index, const cocos2d::Vec2& pos);
	float getObjectRotation(std::uint32_t index);
	void setObjectRotation(std::uint32_t index, float rotation);
	cocos2d::Vec2 getObjectVelocity(std::uint32_t index);
	void setObjectVelocity(std::uint32_t index, const cocos2d::Vec2& vel);

	void setBodyAwake(std::uint32_t index, bool awake);

	void addObjectEvent(std::uint32_t objIndex, const std::string& eventName, const std::string& funcName);

	int getNumPlayers();
	void checkPlayerRespawn(std::uint32_t respawnTime = 0);

	void setTeam(std::uint16_t clientId, int team);
	int getTeam(std::uint16_t clientId);

	std::uint32_t getNumClientsInTeam(int team);
	class CScriptArray* getClientsInTeam(int team);

	void initScore(size_t numTeams);
	void setScore(size_t index, int score);

	void showGameTimer(bool show);
	void setGameTimer(int time);

	void enableInput(const std::string& name, bool enable);

	int randomInt(int min, int max);

	asIScriptEngine* getEngine() const { return mEngine; }

	asIScriptEngine* mEngine;

	Script* mGlobalScript = 0;

	std::vector<asIScriptContext *> mContexts;

	GameSystem* mGame = nullptr;

	std::vector<ScriptObject*> mScriptObjects;
};

#endif