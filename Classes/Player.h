#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Character.h"

#include "2d/CCLabel.h"

class Rope;

class Player : public Character
{
public:
	enum eNetEvent
	{
		NE_TakeDamage,
		NE_LaunchProjectile,
		NE_CollectCoin,
		NE_AttachRope,
		NE_DetachRope,
	};

	struct PlayerState
	{
		std::uint32_t time;
		bool isRunning;
	};

public:
	Player(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs);
	virtual ~Player();

	void initLocalPlayer();
	void initInput();

	bool isLocalPlayer() const;

	std::uint32_t getGameTime();

	virtual void setPosition(const cocos2d::Vec2& pos) override;

	virtual void kill();

	virtual void jump() override;

	void rope();
	void detachRope();

	virtual void update(float deltaTime) override;
	virtual void interpolate(float deltaTime) override;

	virtual void onNetEvent(std::uint8_t eventId) override;
	virtual void onNetEvent(std::uint8_t eventId, NetMessage& msg) override;

	virtual bool serialize(NetMessage& msg) override;

	std::uint16_t mClientId;
	int mTeam = -1;

	Rope* mRope = nullptr;

	float mJumpRopeForce = 20.0f;
	float mJumpTimer = 0.f;
	bool mDoubleJumped = false;

	float mRopeSpeed = 40.f;

	float mNormalAcceleration = 10.f;
	float mAirAcceleration = 7.f;
	float mRopeAcceleration = 15.0f;

	cocos2d::Label* mPlayerNameLabel = nullptr;

	StateHistory<PlayerState> mPlayerState;
	PlayerState mCurrPlayerState;

	cocos2d::RefPtr<cocos2d::Animation> mStandAnimation;
	cocos2d::RefPtr<cocos2d::Animation> mWalkAnimation;
	cocos2d::RefPtr<cocos2d::Animation> mFallAnimation;
};

#endif