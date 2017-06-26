#include "GameMode.h"

#include "GameSystem.h"
#include "Lobby.h"
#include "NetworkManager.h"
#include "LevelScene.h"
#include "Player.h"
#include "Input.h"

GameMode::GameMode(GameSystem* game, const std::string& modeName)
	: mGame(game)
	, mModeName(modeName)
{
	mCurrentState.gameState = GS_NONE;
	mCurrentState.countdown = 0;
	mCurrentState.isCountdownVisible = false;
	mCurrentState.gameOver = false;

	cocos2d::Size winSize = cocos2d::Director::getInstance()->getWinSize();

	mCountdownLabel = cocos2d::Label::createWithTTF("Time:", "fonts/Marker Felt.ttf", 40.0f);
	mCountdownLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
	mCountdownLabel->setColor(cocos2d::Color3B::GRAY);
	mCountdownLabel->setPosition(winSize.width / 2.f, winSize.height / 2.f);
	mCountdownLabel->setVisible(false);
	mGame->mLevelScene->addChild(mCountdownLabel, 15);

	if (mGame->mNetwork->isClient())
	{
		mGameModeState.init(10);
	}

	mGame->mInput->activateAllInGroup("game");
}

GameMode::~GameMode()
{
	mGame->mLevelScene->removeChild(mCountdownLabel);

	for (size_t i = 0; i < mScores.size(); ++i)
	{
		mGame->mLevelScene->removeChild(mScores[i].scoreLabel);
	}
}

void GameMode::setTeam(std::uint16_t clientId, int team)
{
	NetworkManager::PlayerController* pc = mGame->mNetwork->getPlayerController(clientId);
	if (pc)
	{
		pc->team = team;
	}
}

void GameMode::setTeam(Player* player, int team)
{
	setTeam(player->mClientId, team);
}

int GameMode::getTeam(std::uint16_t clientId)
{
	NetworkManager::PlayerController* pc = mGame->mNetwork->getPlayerController(clientId);
	if (pc)
	{
		return pc->team;
	}

	return -1;
}

int GameMode::getTeam(Player* player)
{
	if (!player)
		return -1;
	return getTeam(player->mClientId);
}

size_t GameMode::getNumClientsInTeam(int team)
{
	size_t num = 0;
	auto it = mGame->mLobby->mLobbyUsers.begin();
	auto end = mGame->mLobby->mLobbyUsers.end();
	while (it != end)
	{
		if (it->isConnected)
		{
			NetworkManager::PlayerController* pc = mGame->mNetwork->getPlayerController(it->clientId);
			if (pc && pc->team == team)
				num++;
		}
		++it;
	}
	return num;
}

void GameMode::getClientsInTeam(int team, std::vector<std::uint16_t>& clients)
{
	auto it = mGame->mLobby->mLobbyUsers.begin();
	auto end = mGame->mLobby->mLobbyUsers.end();
	while (it != end)
	{
		if (it->isConnected)
		{
			NetworkManager::PlayerController* pc = mGame->mNetwork->getPlayerController(it->clientId);
			if (pc && pc->team == team)
				clients.push_back(it->clientId);
		}
		++it;
	}
}

void GameMode::checkPlayerRespawn(std::uint32_t respawnTime)
{
	if (!mGame->mNetwork->isServer() || !mGame->mIsGameRunning)
	{
		return;
	}

	for (size_t i = 0; i < mGame->mLobby->mLobbyUsers.size(); ++i)
	{
		const std::uint16_t clientId = mGame->mLobby->mLobbyUsers[i].clientId;

		NetworkManager::PlayerController* pc = mGame->mNetwork->getPlayerController(clientId);
		if (pc)
		{
			if (pc->wantRespawn())
			{
				if (respawnTime > 0)
				{
					if (pc->spectateStartTime == 0)
					{
						pc->spectateStartTime = mGame->mNetwork->getTime();
						continue;
					}
					if (mGame->mNetwork->getTime() - pc->spectateStartTime < respawnTime)
					{
						continue;
					}
				}

				cocos2d::Vec2 spawnPos = getPlayerSpawnSpot(clientId);
				Player* player = mGame->spawnPlayer(mGame->mLobby->mLobbyUsers[i].clientId, spawnPos);
			}
		}
	}
}

void GameMode::initScore(size_t numTeams)
{
	cocos2d::Size winSize = cocos2d::Director::getInstance()->getWinSize();

	for (size_t i = 0; i < mScores.size(); ++i)
	{
		mGame->mLevelScene->removeChild(mScores[i].scoreLabel);
	}

	mScores.resize(numTeams);

	float offset = -(numTeams * 100.f) / 2.f;

	for (size_t i = 0; i < numTeams; ++i)
	{
		mScores[i].scoreLabel = cocos2d::Label::createWithTTF("0", "fonts/Marker Felt.ttf", 40.0f);
		mScores[i].scoreLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
		if (numTeams == 1)
			mScores[i].scoreLabel->setColor(cocos2d::Color3B::GRAY);
		else
		{
			if (i == 0)
				mScores[i].scoreLabel->setColor(cocos2d::Color3B::RED);
			else if (i == 1)
				mScores[i].scoreLabel->setColor(cocos2d::Color3B::BLUE);
			else
				mScores[i].scoreLabel->setColor(cocos2d::Color3B::BLACK);
		}
		mScores[i].scoreLabel->setPosition(winSize.width / 2.f + offset, winSize.height - 30.f);
		mGame->mLevelScene->addChild(mScores[i].scoreLabel, 15);

		offset += 200.f;
	}
}

void GameMode::setScore(size_t index, int score)
{
	if (index >= mScores.size())
		return;

	mScores[index].score = score;
	std::stringstream ss;
	ss << score;
	mScores[index].scoreLabel->setString(ss.str());
}

void GameMode::showCountdown(bool show)
{
	mCountdownLabel->setVisible(show);
}

void GameMode::setCountdown(int time)
{
	std::stringstream ss;
	ss << time;
	mCountdownLabel->setString("Time: " + ss.str());

	mCurrentState.countdown = time;
}

void GameMode::update(float deltaTime)
{
	if (mGame->mNetwork->isClient())
	{
		GameModeState nextState = mCurrentState;
		mGameModeState.getState(mGame->mNetwork->mInterpolationTime, nextState);
		mCurrentState = nextState;

		showCountdown(mCurrentState.isCountdownVisible);
		setCountdown(mCurrentState.countdown);

		if (mCurrentState.gameOver)
			mGame->setGameOver();
	}
}

std::uint32_t GameMode::calcNetPriority(std::uint16_t clientId)
{
	return NET_MAX_PRIORITY;
}

bool GameMode::serialize(NetMessage& msg)
{
	if (msg.mMsgId == NetMessage::MSG_Sync)
	{
		GameModeState nextState = mCurrentState;
		if (msg.isReading())
		{
			if (!mGameModeState.empty())
				nextState = mGameModeState.newest();
		}
		else
		{
			nextState.gameOver = mGame->isGameOver();
			nextState.isCountdownVisible = mCountdownLabel->isVisible();
		}

		msg.serializeUInt(nextState.gameState);
		msg.serializeBool(nextState.isCountdownVisible);
		msg.serializeInt(nextState.countdown);
		msg.serializeBool(nextState.gameOver);

		for (std::int16_t i = 0; i < 4; i++)
		{
			std::int16_t score = std::numeric_limits<std::int16_t>::max();
			if (i < mScores.size())
			{
				score = mScores[i].score;
			}

			msg.serializeShort(score);

			if (msg.isReading())
			{
				if (score != std::numeric_limits<std::int16_t>::max())
				{
					if (i >= mScores.size())
						initScore(i + 1);
					setScore(i, score);
				}
			}
		}

		if (msg.isReading())
		{
			mGameModeState.add(mGame->mNetwork->mSnapServerTime, nextState);
		}
	}

	return true;
}