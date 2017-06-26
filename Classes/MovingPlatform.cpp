#include "MovingPlatform.h"

#include "GameSystem.h"
#include "Physics.h"
#include "Player.h"
#include "NetMessage.h"
#include "NetworkManager.h"
#include "LevelScene.h"

#include "DynamicCrayonRenderer.h"

#include "cocos2d.h"

MovingPlatform::MovingPlatform(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs)
	: GameObject(game)
{
	setNetworkSyncEnable(true);

	mStartPos = pos;
	mEndPos.x = mStartPos.x + spawnArgs.getFloat("ex", 0.f);
	mEndPos.y = mStartPos.y + spawnArgs.getFloat("ey", 0.f);

	mCrayonRenderer = std::make_unique<DynamicCrayonRenderer>();
	
	float width = 2.f;
	float height = 0.5f;
	cocos2d::Vec2 origin;
	mCrayonRenderer->drawSegment(cocos2d::Vec2(origin.x - 32.f * width, origin.y - 32.f * height), cocos2d::Vec2(origin.x - 32.f * width, origin.y + 32.f * height));
	mCrayonRenderer->drawSegment(cocos2d::Vec2(origin.x - 32.f * width, origin.y + 32.f * height), cocos2d::Vec2(origin.x + 32.f * width, origin.y + 32.f * height));
	mCrayonRenderer->drawSegment(cocos2d::Vec2(origin.x + 32.f * width, origin.y + 32.f * height), cocos2d::Vec2(origin.x + 32.f * width, origin.y - 32.f * height));
	mCrayonRenderer->drawSegment(cocos2d::Vec2(origin.x + 32.f * width, origin.y - 32.f * height), cocos2d::Vec2(origin.x - 32.f * width, origin.y - 32.f * height));

	mNode = mCrayonRenderer->getNode();
	setPosition(pos);
	game->addNode(mNode, 5);

	b2BodyDef bodyDef;
	bodyDef.position.Set(pos.x, pos.y);
	bodyDef.type = b2BodyType::b2_kinematicBody;
	bodyDef.angularDamping = 0.6f;
	bodyDef.linearDamping = 0.2f;
	mBody = mGame->mPhysics->world->CreateBody(&bodyDef);

	b2PolygonShape poly;
	poly.SetAsBox(width * mNode->getScaleX(),
		height * mNode->getScaleY());

	b2FixtureDef fd;
	fd.shape = &poly;
	fd.userData = this;
	fd.friction = 0.6f;
	b2Fixture* fix = mBody->CreateFixture(&fd);

	if (mGame->mNetwork->isClient())
		mTransformInterpolation.init(10);
}

MovingPlatform::~MovingPlatform()
{

}

void MovingPlatform::update(float deltaTime)
{
	GameObject::update(deltaTime);

	if (mGame->mNetwork->isClient())
	{
		cocos2d::Vec2 position = getPosition();
		cocos2d::Vec2 vel = cocos2d::Vec2(mBody->GetLinearVelocity().x, mBody->GetLinearVelocity().y);
		mTransformInterpolation.interpolate(mGame->mNetwork->mInterpolationTime, &position, &vel, 0);

		cocos2d::Vec2 currPos = getPosition();

		cocos2d::Vec2 vel2 = (position - currPos);
		mBody->SetLinearVelocity(b2Vec2(vel2.x, vel2.y));

		return;
	}

	cocos2d::Vec2 oldPos = getPosition();
	cocos2d::Vec2 newPos = oldPos;
	cocos2d::Vec2 goalPos;
	cocos2d::Vec2 startPos;

	if (mIsMovingToEnd)
	{
		goalPos = mEndPos;
		startPos = mStartPos;
	}
	else
	{
		goalPos = mStartPos;
		startPos = mEndPos;
	}

	float distStart = (startPos - oldPos).length();

	cocos2d::Vec2 dir = goalPos - oldPos;
	float distance = dir.length();
	dir.normalize();

	float mSlowDownDist = 0.3f;

	float speed = mSpeed;
	float dist = 0.0f;
	if (distStart < mSlowDownDist)
		dist = distStart;
	else
		dist = distance;

	if (dist < mSlowDownDist)
	{
		float value = dist / mSlowDownDist;
		/*if (value < 0.1f)
		value = 0.1f;*/
		speed = mSpeed * value;
		if (speed < 0.1f)
			speed = 0.1f;
	}

	cocos2d::Vec2 delta = dir * speed * deltaTime;
	if (delta.length() < distance)
	{
		newPos += delta;
	}
	else
	{
		newPos = goalPos;
		mIsMovingToEnd = mIsMovingToEnd ? false : true;
	}

	cocos2d::Vec2 vel = (newPos - oldPos) / deltaTime;
	if (vel.lengthSquared() > 0.0f)
		mBody->SetLinearVelocity(b2Vec2(vel.x, vel.y));

	mGame->addDebugLine(getPosition() * Physics::PPM, (getPosition() + vel) * Physics::PPM, cocos2d::Color4F::GREEN);
}

std::uint32_t MovingPlatform::calcNetPriority(std::uint16_t clientId)
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

bool MovingPlatform::serialize(NetMessage& msg)
{
	GameObject::serialize(msg);

	switch (msg.mMsgId)
	{
	case NetMessage::MSG_Sync:
		{
			cocos2d::Vec2 pos = getPosition();
			b2Vec2 vel = mBody->GetLinearVelocity();
			if (msg.isReading() && !mTransformInterpolation.isEmpty())
			{
				const TransformInterpolation::Transform& trans = mTransformInterpolation.newest();
				pos = trans.pos;
				vel = cocos2d2Box2D(trans.vel);
			}

			msg.serializeFloat(pos.x, 100);
			msg.serializeFloat(pos.y, 100);
			msg.serializeFloatShort(vel.x, 100);
			msg.serializeFloatShort(vel.y, 100);

			if (msg.isReading())
			{
				cocos2d::Vec2 newPos = pos + cocos2d::Vec2(vel.x, vel.y);
				mTransformInterpolation.add(mGame->mNetwork->mSnapServerTime, newPos, Box2D2cocos2(vel), 0);
			}

			break;
		}
	}

	return true;
}

#include "GameObjectFactory.h"
REGISTER_GAMEOBJECT(MovingPlatform, GameObject::OT_MovingPlatform, "movingPlatform", true);