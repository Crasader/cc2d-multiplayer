#include "ScriptGameMode.h"

#include "GameSystem.h"
#include "ScriptManager.h"
#include "NetworkManager.h"
#include "Player.h"
#include "PlayerStart.h"

ScriptGameMode::ScriptGameMode(GameSystem* game, const std::string& modeName)
	: GameMode(game, modeName)
{
	if (mGame->mScriptManager->loadFile("gamemode/" + modeName + ".as"))
	{

	}
}

ScriptGameMode::~ScriptGameMode()
{

}

void ScriptGameMode::clientConnected(std::uint16_t clientId)
{
	mGame->mScriptManager->mGlobalScript->call("onClientConnected", clientId);
}

void ScriptGameMode::clientDisconnected(std::uint16_t clientId)
{
	mGame->mScriptManager->mGlobalScript->call("onClientDisconnected", clientId);
}

#include "ScriptFunction.h"

void ScriptGameMode::onStart()
{
	if (!mGame->mNetwork->isServer())
		return;

	mGame->mScriptManager->mGlobalScript->call("onStart");
}

void ScriptGameMode::onAllClientsLoadingFinished()
{
	if (!mGame->mNetwork->isServer())
		return;

	mGame->mScriptManager->mGlobalScript->call("onAllClientsLoadingFinished");
}

cocos2d::Vec2 ScriptGameMode::getPlayerSpawnSpot(std::uint16_t clientId)
{
	cocos2d::Vec2 pos;

	int clientIdInt = clientId;
	if (mGame->mScriptManager->mGlobalScript->call("getPlayerSpawnSpot", clientId))
	{
		pos = *(cocos2d::Vec2*)mGame->mScriptManager->mGlobalScript->getContext()->GetReturnObject();
		mGame->mScriptManager->mGlobalScript->finsishCall();
	}
	else
	{
		std::vector<PlayerStart*> playerStarts;
		mGame->findObjects(playerStarts);

		if (playerStarts.size() >= 1)
		{
			int randomNumber = cocos2d::RandomHelper::random_int(0, (int)playerStarts.size() - 1);
			pos = playerStarts[randomNumber]->getPosition();
		}
	}

	return pos;
}

void ScriptGameMode::onSpawnPlayer(Player* player)
{
	mGame->mScriptManager->mGlobalScript->call("onSpawnPlayer", (GameObject*)player);
}

void ScriptGameMode::onCharacterDestroyed(Character* character)
{
	mGame->mScriptManager->mGlobalScript->call("onCharacterDestroyed", character->getLocalIndex());
}

void ScriptGameMode::update(float deltaTime)
{
	GameMode::update(deltaTime);

	mGame->mScriptManager->mGlobalScript->call("update", deltaTime);
}

bool ScriptGameMode::serialize(NetMessage& msg)
{

	//mGame->mScriptManager->mGlobalScript->call("serialize", msg);

	return GameMode::serialize(msg);
}