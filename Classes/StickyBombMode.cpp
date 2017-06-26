#include "StickyBombMode.h"

#include "GameSystem.h"
#include "NetworkManager.h"
#include "Lobby.h"

#include "Player.h"
#include "TriggerBox.h"
#include "TargetPoint.h"
#include "PlayerStart.h"

#include "StickyBomb.h"

StickyBombMode::StickyBombMode(GameSystem* game, const std::string& modeName)
	: GameMode(game, modeName)
{

}

StickyBombMode::~StickyBombMode()
{

}

void StickyBombMode::onStart()
{
	mPlayerStarts.clear();
	mGame->findObjects(mPlayerStarts);
}

void StickyBombMode::onAllClientsLoadingFinished()
{
	checkPlayerRespawn();

	mResetTimer = 3.f;
}

cocos2d::Vec2 StickyBombMode::getPlayerSpawnSpot(std::uint16_t clientId)
{
	cocos2d::Vec2 spawnSpot;

	if (mPlayerStarts.size() == 0)
	{
		mGame->findObjects(mPlayerStarts);
	}

	if (mPlayerStarts.size() >= 1)
	{
		int randomNumber = cocos2d::RandomHelper::random_int(0, (int)mPlayerStarts.size() - 1);

		spawnSpot = mPlayerStarts[randomNumber]->getPosition();
		mPlayerStarts.pop_back();
	}

	return spawnSpot;
}

void StickyBombMode::onBombDestroyed(GameObject* owner)
{
	if (mStickyBomb == owner)
	{
		mStickyBomb = nullptr;

		mResetTimer = 3.f;
	}
}

void StickyBombMode::onPlayerContactEnter(GameObject* owner, GameObject* obj, const ContactInfo& contact)
{
	if (mContactTimer > 0.f)
		return;

	Player* otherPlayer = dynamic_cast<Player*>(obj);
	if (otherPlayer)
	{
		if (mStickyBomb->mAttachedPlayer)
		{
			mStickyBomb->mAttachedPlayer->mOnContactEnter.remove(this);
		}

		mStickyBomb->attachObject(otherPlayer);
		otherPlayer->mOnContactEnter.add(this, &StickyBombMode::onPlayerContactEnter);

		mContactTimer = 0.5f;
	}
}

void StickyBombMode::onCharacterDestroyed(class Character* character)
{
	if (!mGame->mNetwork->isServer())
		return;

	if (mStickyBomb && mStickyBomb->mAttachedPlayer == character)
	{
		mStickyBomb->attachObject(nullptr);
		mStickyBomb->destroyMe();
		mStickyBomb = nullptr;

		mResetTimer = 3.f;
	}

	if (mGame->calcPlayerNum() <= 1)
	{
		std::vector<Player*> players;
		mGame->findObjects<Player>(players);
		for (Player* player : players)
		{
			mGame->scoreUser(player->mClientId);
		}

		mGame->setGameOver();
	}
}

void StickyBombMode::update(float deltaTime)
{
	GameMode::update(deltaTime);

	if (!mGame->mNetwork->isServer() || mGame->isGameOver())
		return;

	if (mContactTimer > 0.f)
		mContactTimer -= deltaTime;

	if (!mStickyBomb && mResetTimer > 0.f)
	{
		mResetTimer -= deltaTime;

		showCountdown(true);
		setCountdown(mResetTimer + 1);

		if (mResetTimer <= 0.f)
		{
			showCountdown(false);

			Parameters p;
			cocos2d::Vec2 pos(0, 0);
			mStickyBomb = static_cast<StickyBomb*>(mGame->spawnGameObject("stickybomb", pos, p));
			if (mStickyBomb)
			{
				mStickyBomb->mOnDestroyed.add(this, &StickyBombMode::onBombDestroyed);

				std::vector<Player*> players;
				mGame->findObjects(players);

				if (players.size() > 0)
				{
					int randomPlayerNumber = cocos2d::RandomHelper::random_int(0, (int)players.size() - 1);

					mStickyBomb->attachObject(players[randomPlayerNumber]);
					players[randomPlayerNumber]->mOnContactEnter.add(this, &StickyBombMode::onPlayerContactEnter);
				}
			}
		}
	}
}

#include "GameModeFactory.h"
REGISTER_GAMEMODE(StickyBombMode, "stickyBomb")