#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include "GameObject.h"

#include "MulticastDelegate.h"
#include "TransformInterpolation.h"

#include "2d/CCAnimation.h"
#include "2d/CCAction.h"
#include "2d/CCActionInterval.h"

class Character : public GameObject
{
public:
	enum MoveState
	{
		MS_NONE,
		MS_Walking,
		MS_Falling,
	};

	struct CharacterState
	{
		std::uint32_t time;
		float moveSpeed;
		cocos2d::Vec2 moveVec;
		bool isLookRight;
		std::uint16_t moveState;
	};

public:
	Character(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs);
	virtual ~Character();

	void createBody(float radius, float height);

	float getMovingSpeed();

	virtual void jump();

	virtual void onContactPostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

	virtual void kill();

	virtual void changeDirection(bool right);

	void changeAnimation(cocos2d::Animation* anim);

	void checkGround();

	virtual void update(float deltaTime) override;
	virtual void interpolate(float deltaTime);

	virtual std::uint32_t calcNetPriority(std::uint16_t clientId) override;
	virtual bool serialize(NetMessage& msg) override;

	cocos2d::RefPtr<cocos2d::Animate> mAnimate;
	cocos2d::RefPtr<cocos2d::Action> mAnimAction;
	cocos2d::RefPtr<cocos2d::Speed> mSpeedAction;

	b2Fixture* mHeadFixture = nullptr;
	b2Fixture* mBodyFixture = nullptr;
	b2Fixture* mFootFixture = nullptr;

	float mCharacterHeight;
	float mCharacterRadius;

	cocos2d::Vec2 mBaseVelocity;

	float mMaxSpeed = 8.0f;
	float mWalkSpeed = 8.0f;
	float mRunSpeed = 12.0f;
	float mAcceleration = 300.0f;
	float mJumpForce = 160.0f;

	bool mOnGround = false;
	b2Vec2 mGroundNormal;
	b2Body* mGoundBody = nullptr;

	float mWalkGroundAngle = 0.7071f; // 45 Grad
	float mJumpGroundAngle = 0.5f; // 60 Grad
	bool mOnJumpGround = false;

	TransformInterpolation mTransformInterpolation;
	StateHistory<CharacterState> mCharacterState;
	CharacterState mCurrCharacterState;
};

#endif