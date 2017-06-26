#ifndef __GAMEOBJECTFACTORY_H__
#define __GAMEOBJECTFACTORY_H__

#include <unordered_map>
#include "math/Vec2.h"

class GameObject;
class GameSystem;
class Parameters;

class GameObjectFactoryBase
{
public:
	GameObjectFactoryBase(uint32_t typeId, const std::string& className, bool createOnlyOnServer);
	virtual GameObject* create(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs) = 0;

	int mTypeId;
	std::string mClassName;
	bool mCreateOnlyOnServer;
};

template <class T>
class GameObjectFactory : public GameObjectFactoryBase
{
public:
	GameObjectFactory(uint32_t typeId, const std::string& className, bool createOnlyOnServer)
		: GameObjectFactoryBase(typeId, className, createOnlyOnServer)
	{}
	T* create(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs) override
	{
		T* obj = new T(game, pos, spawnArgs);
		obj->mTypeId = mTypeId;
		return obj;
	}
};

class GameObjectDatabase
{
public:
	static void registerObject(uint32_t typeId, const std::string& className, GameObjectFactoryBase* factory);

	static GameObject* create(uint32_t typeId, GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs);
	static GameObject* create(const std::string& className, GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs);

	static GameObjectFactoryBase* getFactory(const std::string& className);
	static GameObjectFactoryBase* getFactory(uint32_t typeId);

	static GameObjectDatabase* getInstance()
	{
		static GameObjectDatabase* instance = 0;
		if (!instance)
		{
			instance = new GameObjectDatabase();
		}
		return instance;
	}

	std::unordered_map<uint32_t, GameObjectFactoryBase*> mIdFactorys;
	std::unordered_map<std::string, GameObjectFactoryBase*> mNameFactorys;
};


#define REGISTER_GAMEOBJECT(Class, ClassTypeId, ClassName, CreateOnlyOnServer) \
	GameObjectFactory<Class> gGameObjectFactory##Class(ClassTypeId, ClassName, CreateOnlyOnServer);

#endif