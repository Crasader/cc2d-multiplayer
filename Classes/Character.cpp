#include "Character.h"

#include "GameSystem.h"
#include "Physics.h"
#include "NetworkManager.h"

#include "NetMessage.h"
#include "GameMode.h"

Character::Character(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs)
	: GameObject(game)
{
	setNetworkSyncEnable(true);
}

Character::~Character()
{
	mGame->mGameMode->onCharacterDestroyed(this);
}

void Character::createBody(float radius, float height)
{
	mCharacterHeight = height;
	mCharacterRadius = radius;

	if (!mBody)
	{
		b2BodyDef bodyDef;
		bodyDef.type = b2BodyType::b2_dynamicBody;
		bodyDef.position.x = mNode->getPosition().x / 32.0f;
		bodyDef.position.y = mNode->getPosition().y / 32.0f;
		bodyDef.fixedRotation = true;
		mBody = mGame->mPhysics->world->CreateBody(&bodyDef);
	}

	if (height > radius)
	{
		b2CircleShape circle;
		circle.m_radius = radius;
		circle.m_p = b2Vec2(0, (height - radius));
		if (mHeadFixture)
			mBody->DestroyFixture(mHeadFixture);
		mHeadFixture = mBody->CreateFixture(&circle, 1.0f);
		mHeadFixture->SetFriction(0.0f);
		mHeadFixture->SetUserData(this);

		b2PolygonShape poly;
		poly.SetAsBox(radius - 0.01f, height - radius, b2Vec2(0, 0), 0.0f);
		if (mBodyFixture)
			mBody->DestroyFixture(mBodyFixture);
		mBodyFixture = mBody->CreateFixture(&poly, 10.0f);
		mBodyFixture->SetFriction(0.0f);
		mBodyFixture->SetUserData(this);
	}
	else
	{
		if (mHeadFixture)
		{
			mBody->DestroyFixture(mHeadFixture);
			mHeadFixture = 0;
		}
		if (mBodyFixture)
		{
			mBody->DestroyFixture(mBodyFixture);
			mBodyFixture = 0;
		}
	}

	b2CircleShape circle;
	circle.m_radius = radius;
	circle.m_p = b2Vec2(0, -(height - radius));
	if (mFootFixture)
		mBody->DestroyFixture(mFootFixture);
	mFootFixture = mBody->CreateFixture(&circle, (mBodyFixture ? 5.0f : 10.f));
	mFootFixture->SetFriction(0.0f);
	mFootFixture->SetUserData(this);

	b2Filter filter;
	if (mBodyFixture != nullptr) {
		filter = mBodyFixture->GetFilterData();
		filter.categoryBits = CollisionGroup::CHARACTER;
		filter.maskBits = CollisionGroup::ALL & ~(CollisionGroup::ROPE);
		mBodyFixture->SetFilterData(filter);
	}

	if (mHeadFixture != nullptr) {
		filter = mHeadFixture->GetFilterData();
		filter.categoryBits = CollisionGroup::CHARACTER;
		filter.maskBits = CollisionGroup::ALL & ~(CollisionGroup::ROPE);
		mHeadFixture->SetFilterData(filter);
	}

	filter = mFootFixture->GetFilterData();
	filter.categoryBits = CollisionGroup::CHARACTER;
	filter.maskBits = CollisionGroup::ALL & ~(CollisionGroup::ROPE);
	mFootFixture->SetFilterData(filter);
}

float Character::getMovingSpeed()
{
	return mCurrCharacterState.moveSpeed;
}

void Character::jump()
{
	if (mOnJumpGround && mCurrCharacterState.moveState == MS_Walking)
	{
		mCurrCharacterState.moveState = MS_Falling;
		mBody->ApplyLinearImpulse(b2Vec2(0, mJumpForce), mBody->GetWorldCenter(), true);
	}
}

void Character::onContactPostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
	if (impulse->normalImpulses[0] > 500.0f)
	{
		kill();
	}
}

void Character::kill()
{
	if (!mGame->mNetwork->isServer())
		return;

	destroyMe();
}

void Character::changeDirection(bool right)
{
	if (mCurrCharacterState.isLookRight == right)
		return;

	if (right)
	{
		mCurrCharacterState.isLookRight = true;
		mNode->setScaleX(std::abs(mNode->getScaleX()) * 1.0f);
	}
	else
	{
		mCurrCharacterState.isLookRight = false;
		mNode->setScaleX(std::abs(mNode->getScaleX()) * -1.0f);
	}
}

void Character::changeAnimation(cocos2d::Animation* anim)
{
	cocos2d::Sprite* sprite = dynamic_cast<cocos2d::Sprite*>(mNode.get());
	if (!sprite)
		return;

	if (mAnimate)
	{
		if (mAnimate->getAnimation() == anim)
			return;

		sprite->stopAction(mAnimAction);
	}

	mAnimate = cocos2d::Animate::create(anim);
	if (!mSpeedAction.get())
	{
		mSpeedAction = cocos2d::Speed::create(cocos2d::RepeatForever::create(mAnimate), 1.0f);
	}
	else
	{
		mSpeedAction->setInnerAction(cocos2d::RepeatForever::create(mAnimate));
	}
	mAnimAction = sprite->runAction(mSpeedAction);
}

void Character::checkGround()
{
	mOnGround = false;
	mOnJumpGround = false;
	mGoundBody = nullptr;

	b2ContactEdge* contactEdge = mBody->GetContactList();
	while (contactEdge)
	{
		if (contactEdge->contact->IsTouching() && 
			!contactEdge->contact->GetFixtureA()->IsSensor() && !contactEdge->contact->GetFixtureB()->IsSensor())
		{
			b2WorldManifold manifold;
			contactEdge->contact->GetWorldManifold(&manifold);
			b2Vec2 normal = manifold.normal;

			GameObject* objA = static_cast<GameObject*>(contactEdge->contact->GetFixtureA()->GetUserData());
			if (objA == this)
			{
				normal *= -1.0f;
			}

			if (normal.y > mWalkGroundAngle)
			{
				mOnGround = true;
				mGroundNormal = normal;
				mGoundBody = contactEdge->other;
			}
			if (normal.y > mJumpGroundAngle)
			{
				mOnJumpGround = true;
			}
		}

		contactEdge = contactEdge->next;
	}
}

void Character::update(float deltaTime)
{
	GameObject::update(deltaTime);

	checkGround();

	const b2Vec2 vel = mBody->GetLinearVelocity();
	const bool overMax = abs(vel.x) > mMaxSpeed;
	const bool wrongDir = (mCurrCharacterState.moveVec.x * vel.x) < 0;

	bool changed = false;

	if (mOnGround)
	{
		if (mCurrCharacterState.moveState == MS_Falling)
		{
			mCurrCharacterState.moveState = MS_Walking;
			changed = true;
		}
	}
	else
	{
		mCurrCharacterState.moveState = MS_Falling;
		mBaseVelocity.setZero();
	}

	if (mCurrCharacterState.moveState == MS_Walking || mCurrCharacterState.moveState == MS_Falling)
	{
		float acceleration = mAcceleration * mCurrCharacterState.moveVec.x;
		const bool isZeroAcc = acceleration == 0.f;

		cocos2d::Vec2 velocity = Box2D2cocos2(mBody->GetLinearVelocity()) - mBaseVelocity;
		float friction = 1.0f;

		if (mCurrCharacterState.moveState != MS_Falling)
		{
			if (isZeroAcc || overMax)
			{
				if (!velocity.isZero())
				{
					// braking
					cocos2d::Vec2 revAccel = (-(mAcceleration * 2.f) * velocity.getNormalized());

					cocos2d::Vec2 oldVel = velocity;
					velocity = velocity + ((-friction) * velocity + revAccel) * deltaTime;
					if ((velocity.x * oldVel.x) < 0.f)
					{
						velocity.x = 0.f;
					}
				}
			}
			else if (!isZeroAcc && wrongDir)
			{
				velocity -= velocity * (friction * deltaTime);
			}
		}

		if (!overMax || wrongDir)
		{
			velocity.x += acceleration * deltaTime;
		}

		velocity.y = mBody->GetLinearVelocity().y;

		if (mCurrCharacterState.moveVec.x > 0)
		{
			changeDirection(true);
		}
		else if (mCurrCharacterState.moveVec.x < 0)
		{
			changeDirection(false);
		}

		if (mCurrCharacterState.moveState == MS_Walking)
		{
			if (mOnGround && mGoundBody && std::abs(mGoundBody->GetLinearVelocity().x) > 0.f)
			{
				mBaseVelocity.x = mGoundBody->GetLinearVelocity().x;;
				velocity.x += mGoundBody->GetLinearVelocity().x;
			}
			else
				mBaseVelocity.setZero();
		}

		mBody->SetLinearVelocity(cocos2d2Box2D(velocity));
	}
}

void Character::interpolate(float deltaTime)
{

}

std::uint32_t Character::calcNetPriority(std::uint16_t clientId)
{
	NetworkManager::PlayerController* controller = mGame->mNetwork->getPlayerController(clientId);
	if (controller)
	{
		if (controller->hasPlayer())
		{
			float dist = getPosition().distance(static_cast<GameObject*>(controller->player)->getPosition());
			if (dist < mGame->mNetwork->getRelevantDistance())
				return NET_MAX_PRIORITY;
			return std::max((int)((1.f - (dist / (mGame->mNetwork->getRelevantDistance() * 2.f))) * NET_MAX_PRIORITY), 1);
		}
	}
	return NET_MAX_PRIORITY;
}

bool Character::serialize(NetMessage& msg)
{
	GameObject::serialize(msg);

	switch (msg.mMsgId)
	{
	case NetMessage::MSG_Sync:
		{
			cocos2d::Vec2 pos = getPosition();
			b2Vec2 vel = mBody->GetLinearVelocity();

			CharacterState nextState = mCurrCharacterState;

			if (msg.isReading())
			{
				if (mTransformInterpolation.isEmpty())
					mTransformInterpolation.init(10);
				else
				{
					const TransformInterpolation::Transform& trans = mTransformInterpolation.newest();
					pos = trans.pos;
					vel = cocos2d2Box2D(trans.vel);
				}

				if (mCharacterState.empty())
					mCharacterState.init(10);
				else
					nextState = mCharacterState.newest();
			}

			msg.serializeFloat(pos.x, 100);
			msg.serializeFloat(pos.y, 100);
			msg.serializeFloatShort(vel.x, 100);
			msg.serializeFloatShort(vel.y, 100);
			msg.serializeFloatShort(nextState.moveSpeed, 100);
			msg.serializeUShort(nextState.moveState);
			msg.serializeFloatShort(nextState.moveVec.x, 100);
			msg.serializeBool(nextState.isLookRight);

			if (msg.isReading())
			{
				std::uint32_t currTime = mGame->mNetwork->mSnapServerTime;

				mTransformInterpolation.add(currTime, pos, Box2D2cocos2(vel), 0);
				mCharacterState.add(currTime, nextState);

				mGame->addDebugLine(Physics::PPM * pos + cocos2d::Vec2(0, -10), Physics::PPM * pos + cocos2d::Vec2(0, 10), cocos2d::Color4F::RED);
				mGame->addDebugLine(Physics::PPM * pos + cocos2d::Vec2(-10, 0), Physics::PPM * pos + cocos2d::Vec2(10, 0), cocos2d::Color4F::RED);
			}

			break;
		}
	}

	return true;
}