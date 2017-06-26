#include "GameModeFactory.h"

#include "GameSystem.h"
#include "GameMode.h"

#include "ScriptGameMode.h"


GameModeFactoryBase::GameModeFactoryBase(const std::string& className)
{
	GameModeDatabase::registerMode(className, this);
}

#include "json/document.h"
#include "json/filestream.h"

void GameModeDatabase::loadModes()
{
	FILE * pFile = fopen("modes.json", "rb");
	rapidjson::FileStream is(pFile);

	rapidjson::Document d;
	d.ParseStream(is);

	rapidjson::Value& maps = d["modes"];
	if (maps.IsArray())
	{
		for (rapidjson::SizeType i = 0; i < maps.Size(); i++)
		{
			rapidjson::Value& mapValue = maps[i];

			std::string modeName = mapValue["name"].GetString();

			registerMode(modeName, new GameModeFactory<ScriptGameMode>(modeName));
		}
	}

	fclose(pFile);
}

void GameModeDatabase::registerMode(const std::string& className, GameModeFactoryBase* factory)
{
	if (getInstance()->mNameFactorys.find(className) != getInstance()->mNameFactorys.end())
	{
		return;
	}

	factory->mClassName = className;

	getInstance()->mNameFactorys.insert(std::make_pair(className, factory));
}


std::unique_ptr<GameMode> GameModeDatabase::create(const std::string& className, GameSystem* game)
{
	auto it = getInstance()->mNameFactorys.find(className);
	if (it != getInstance()->mNameFactorys.end())
	{
		return it->second->create(game);
	}
	return 0;
}

GameModeFactoryBase* GameModeDatabase::getFactory(const std::string& className)
{
	auto it = getInstance()->mNameFactorys.find(className);
	if (it != getInstance()->mNameFactorys.end())
	{
		return it->second;
	}
	return 0;
}