#ifndef __STICKYBOMB_H__
#define __STICKYBOMB_H__

#include "GameObject.h"

#include "TransformInterpolation.h"

#include "cocos2d.h"

class Player;

class StickyBomb : public GameObject
{
public:
	enum eNetEvent
	{
		NE_Explode,
	};

	struct StickyBombState
	{
		std::uint32_t time;
		float bombTimer;
		std::uint32_t attachedNetId;
	};

public:
	StickyBomb(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs);
	virtual ~StickyBomb();

	void explode();

	void onDestroyedAttachedObject(GameObject* obj);

	virtual void update(float deltaTime) override;

	virtual void onNetEvent(std::uint8_t eventId) override;
	virtual void onNetEvent(std::uint8_t eventId, NetMessage& msg) override;

	virtual bool serialize(NetMessage& msg) override;

	virtual void attachObject(Player* player);

	bool mHasBombTimer = true;

	cocos2d::RefPtr<cocos2d::Speed> mSpeedAction;

	Player* mAttachedPlayer = nullptr;
	cocos2d::Vec2 mOffestPos;

	StateHistory<StickyBombState> mStickyBombState;
	StickyBombState mCurrState;
};

#endif