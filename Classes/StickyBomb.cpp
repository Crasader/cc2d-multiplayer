#include "StickyBomb.h"

#include "GameSystem.h"
#include "NetworkManager.h"
#include "NetMessage.h"
#include "NetworkManager.h"

#include "Physics.h"
#include "Input.h"

#include "Player.h"

StickyBomb::StickyBomb(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs)
	: GameObject(game)
{
	setNetworkSyncEnable(true);

	mCurrState.attachedNetId = NetworkManager::InvalidNetId;
	mCurrState.bombTimer = cocos2d::RandomHelper::random_real(20.f, 30.f);

	auto sprite = cocos2d::Sprite::create();
	mNode = sprite;
	setPosition(pos);
	game->addNode(sprite, 7);

	mOffestPos = cocos2d::Vec2(0.f, 2.3f);

	auto animation = cocos2d::Animation::create();

	animation->addSpriteFrameWithFile("Bomb1.png");
	animation->addSpriteFrameWithFile("Bomb2.png");

	animation->setDelayPerUnit(1.5f / 14.0f);
	animation->setRestoreOriginalFrame(true);

	auto animate = cocos2d::Animate::create(animation);

	mSpeedAction = cocos2d::Speed::create(cocos2d::RepeatForever::create(animate), 0.3f);
	sprite->runAction(mSpeedAction);

	mHasBombTimer = spawnArgs.getBool("hasBombTimer", true);

	if (mGame->mNetwork->isClient())
	{
		mStickyBombState.init(10);
	}
}

StickyBomb::~StickyBomb()
{
}

void StickyBomb::attachObject(Player* player)
{
	mAttachedPlayer = player;

	if (mAttachedPlayer)
	{
		mCurrState.attachedNetId = mAttachedPlayer->mNetId;

		mAttachedPlayer->mOnDestroyed.add(this, &StickyBomb::onDestroyedAttachedObject);
	}
	else
	{
		mCurrState.attachedNetId = NetworkManager::InvalidNetId;
	}
}

void StickyBomb::explode()
{
	if (mIsDestroyed)
		return;

	if (!mGame->mNetwork->isServer())
		return;

	if (mAttachedPlayer)
	{
		mAttachedPlayer->kill();
		attachObject(nullptr);
	}

	destroyMe();
}

void StickyBomb::onDestroyedAttachedObject(GameObject* obj)
{
	if (mAttachedPlayer == obj)
	{
		attachObject(nullptr);
	}
}

void StickyBomb::update(float deltaTime)
{
	GameObject::update(deltaTime);

	if (mGame->mNetwork->isClient())
	{
		StickyBombState nextState = mCurrState;
		mStickyBombState.getState(mGame->mNetwork->getInterpolationTime(), nextState);

		if (nextState.attachedNetId != mCurrState.attachedNetId)
		{
			attachObject(dynamic_cast<Player*>(mGame->mNetwork->getObject(nextState.attachedNetId)));
		}
		mCurrState = nextState;
	}

	if (mGame->mNetwork->isServer())
	{
		if (mAttachedPlayer)
		{
			cocos2d::Vec2 pos = mAttachedPlayer->getPosition();
			setPosition(pos + mOffestPos);
		}
	}
	else if (mGame->mNetwork->isClient())
	{
		GameObject* obj = dynamic_cast<GameObject*>(mGame->mNetwork->getObject(mCurrState.attachedNetId));
		if (obj)
		{
			cocos2d::Vec2 pos = obj->getPosition();
			setPosition(pos + mOffestPos);
		}
	}

	float speed = 1.0f - (mCurrState.bombTimer / 3.0f);
	speed = std::max(speed, 0.3f);

	mSpeedAction->setSpeed(speed);

	if (!mGame->mNetwork->isServer())
		return;

	if (mHasBombTimer)
	{
		mCurrState.bombTimer -= deltaTime;
		if (mCurrState.bombTimer <= 0.0f)
		{
			explode();
		}
	}
}

void StickyBomb::onNetEvent(std::uint8_t eventId)
{
	if (eventId == NE_Explode)
	{
		explode();
	}
}

void StickyBomb::onNetEvent(std::uint8_t eventId, NetMessage& msg)
{
	if (eventId == 0)
	{
		std::uint32_t netId = NetworkManager::InvalidNetId;
		msg.readUInt(netId);

		GameObject* obj = dynamic_cast<GameObject*>(mGame->mNetwork->getObject(netId));
		if (obj)
		{
			attachObject(dynamic_cast<Player*>(obj));
		}
	}
}

bool StickyBomb::serialize(NetMessage& msg)
{
	GameObject::serialize(msg);

	switch (msg.mMsgId)
	{
	case NetMessage::MSG_Sync:
		{
			StickyBombState nextState = mCurrState;
			if (msg.isReading() && !mStickyBombState.empty())
			{
				nextState = mStickyBombState.newest();
			}

			msg.serializeUInt(nextState.attachedNetId);
			msg.serializeFloatShort(nextState.bombTimer, 100);

			if (msg.isReading())
			{
				mStickyBombState.add(mGame->mNetwork->mSnapServerTime, nextState);
			}

			break;
		}
	}

	return true;
}

#include "GameObjectFactory.h"
REGISTER_GAMEOBJECT(StickyBomb, GameObject::OT_StickyBomb, "stickybomb", true);