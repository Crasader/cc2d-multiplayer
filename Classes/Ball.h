#ifndef __BALL_H__
#define __BALL_H__

#include "GameObject.h"

#include "Box2D\Box2D.h"

#include "TransformInterpolation.h"

class Ball : public GameObject
{
public:
	struct BallState
	{
		std::uint32_t time;
		bool isVisible;
		bool enableCharacterCollision;
	};

public:
	Ball(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs);
	virtual ~Ball();

	void createBody();

	void enableCharacterCollision(bool enable);

	virtual void onContactEnter(GameObject* obj, const ContactInfo& contact) override;

	virtual void update(float deltaTime) override;

	void interpolate(float deltaTime);

	virtual std::uint32_t calcNetPriority(std::uint16_t clientId) override;
	virtual bool serialize(NetMessage& msg) override;

	b2CircleShape mBallShape;
	b2BodyDef mBodyDef;
	b2FixtureDef mFixtureDef;
	b2Fixture* mFixture = nullptr;

	b2Vec2 mImpactImpulse;
	bool mUseCenterImpulse;

	bool mIsKinematicOnClient;

	TransformInterpolation mTransformInterpolation;
	StateHistory<BallState> mBallState;
	BallState mCurrentState;
};

#endif