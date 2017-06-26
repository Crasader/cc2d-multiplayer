#include "GameObjectFactory.h"

GameObjectFactoryBase::GameObjectFactoryBase(uint32_t typeId, const std::string& className, bool createOnlyOnServer)
	: mCreateOnlyOnServer(createOnlyOnServer)
{
	GameObjectDatabase::registerObject(typeId, className, this);
}

void GameObjectDatabase::registerObject(uint32_t typeId, const std::string& className, GameObjectFactoryBase* factory)
{
	if (getInstance()->mIdFactorys.find(typeId) != getInstance()->mIdFactorys.end())
	{
		return;
	}

	if (getInstance()->mNameFactorys.find(className) != getInstance()->mNameFactorys.end())
	{
		return;
	}

	factory->mTypeId = typeId;
	factory->mClassName = className;

	getInstance()->mIdFactorys.insert(std::make_pair(typeId, factory));
	getInstance()->mNameFactorys.insert(std::make_pair(className, factory));
}

GameObject* GameObjectDatabase::create(uint32_t typeId, GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs)
{
	auto it = getInstance()->mIdFactorys.find(typeId);
	if (it != getInstance()->mIdFactorys.end())
	{
		return it->second->create(game, pos, spawnArgs);
	}
	return 0;
}

GameObject* GameObjectDatabase::create(const std::string& className, GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs)
{
	auto it = getInstance()->mNameFactorys.find(className);
	if (it != getInstance()->mNameFactorys.end())
	{
		return it->second->create(game, pos, spawnArgs);
	}
	return 0;
}

GameObjectFactoryBase* GameObjectDatabase::getFactory(const std::string& className)
{
	auto it = getInstance()->mNameFactorys.find(className);
	if (it != getInstance()->mNameFactorys.end())
	{
		return it->second;
	}
	return 0;
}

GameObjectFactoryBase* GameObjectDatabase::getFactory(uint32_t typeId)
{
	auto it = getInstance()->mIdFactorys.find(typeId);
	if (it != getInstance()->mIdFactorys.end())
	{
		return it->second;
	}
	return 0;
}