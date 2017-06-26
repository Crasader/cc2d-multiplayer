#ifndef __GAMEMODEFACTORY_H__
#define __GAMEMODEFACTORY_H__

#include <string>
#include <unordered_map>
#include <memory>

class GameMode;
class GameSystem;

class GameModeFactoryBase
{
public:
	GameModeFactoryBase(const std::string& className);
	virtual std::unique_ptr<GameMode> create(GameSystem* game) = 0;

	std::string mClassName;
};

template <class T>
class GameModeFactory : public GameModeFactoryBase
{
public:
	GameModeFactory(const std::string& className)
		: GameModeFactoryBase(className)
	{}
	std::unique_ptr<GameMode> create(GameSystem* game) override
	{
		return std::make_unique<T>(game, mClassName);
	}
};

class GameModeDatabase
{
public:

	void loadModes();

	static void registerMode(const std::string& className, GameModeFactoryBase* factory);

	static std::unique_ptr<GameMode> create(const std::string& className, GameSystem* game);

	static GameModeFactoryBase* getFactory(const std::string& className);

	static GameModeDatabase* getInstance()
	{
		static GameModeDatabase* instance = 0;
		if (!instance)
		{
			instance = new GameModeDatabase();
			instance->loadModes();
		}
		return instance;
	}

	std::unordered_map<std::string, GameModeFactoryBase*> mNameFactorys;
};

#define REGISTER_GAMEMODE(Class, ClassName) \
GameModeFactory<Class> gModeObjectFactory##Class(ClassName);

#endif