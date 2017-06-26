#include "Ball.h"

#include "GameSystem.h"
#include "Physics.h"
#include "NetworkManager.h"
#include "NetMessage.h"
#include "Character.h"

#include "cocos2d.h"

#include "ChartPanel.h"

Ball::Ball(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs)
	: GameObject(game)
{
	setNetworkSyncEnable(true);

	std::string filename = spawnArgs.getString("sprite", "");
	cocos2d::Sprite* sprite = nullptr;
	if (filename.empty())
		sprite = cocos2d::Sprite::create();
	else
		sprite = cocos2d::Sprite::create(filename);
	float scaleX = spawnArgs.getFloat("scaleX", 1.f);
	float scaleY = spawnArgs.getFloat("scaleY", 1.f);
	sprite->setScale(scaleX, scaleY);
	mNode = sprite;
	setPosition(pos);
	game->addNode(sprite, 3);

	mBallShape.m_radius = spawnArgs.getFloat("radius", 0.24f);
	mBallShape.m_p.x = spawnArgs.getFloat("offsetX", 0.f);
	mBallShape.m_p.y = spawnArgs.getFloat("offsetY", 0.f);

	mBodyDef.position.Set(pos.x, pos.y);
	mBodyDef.type = b2BodyType::b2_dynamicBody;
	mBodyDef.angularDamping = spawnArgs.getFloat("angularDamping", 0.1f);
	mBodyDef.linearDamping = spawnArgs.getFloat("linearDamping", 0.6f);
	mBodyDef.allowSleep = true;

	mFixtureDef.density = spawnArgs.getFloat("density", 1.0f);
	mFixtureDef.restitution = spawnArgs.getFloat("restitution", 0.8f);
	mFixtureDef.userData = this;

	mMaxVelocity = spawnArgs.getFloat("maxVelocity", 10.f);

	mImpactImpulse.x = spawnArgs.getFloat("impulseX", 0.2f);
	mImpactImpulse.y = spawnArgs.getFloat("impulseY", 0.5f);
	mUseCenterImpulse = spawnArgs.getBool("centerImpulse", false);

	mIsKinematicOnClient = spawnArgs.getBool("isKinematicOnClient", false);

	if (mGame->mNetwork->isServer())
	{
		createBody();
	}

	if (mGame->mNetwork->isClient())
	{
		mTransformInterpolation.init(10);
		mBallState.init(10);
	}
}

Ball::~Ball()
{

}

void Ball::createBody()
{
	const cocos2d::Vec2 pos = getPosition();

	if (mIsKinematicOnClient && mGame->mNetwork->isClient())
		mBodyDef.type = b2BodyType::b2_kinematicBody;

	mBody = mGame->mPhysics->world->CreateBody(&mBodyDef);

	mFixtureDef.shape = &mBallShape;
	mFixture = mBody->CreateFixture(&mFixtureDef);
}

void Ball::enableCharacterCollision(bool enable)
{
	b2Filter filter = mFixture->GetFilterData();
	if (enable)
	{
		filter.maskBits |= (CollisionGroup::CHARACTER);
	}
	else
	{
		filter.maskBits &= ~(CollisionGroup::CHARACTER);
	}
	mFixture->SetFilterData(filter);

	mCurrentState.enableCharacterCollision = enable;
}

void Ball::onContactEnter(GameObject* obj, const ContactInfo& contact)
{
	GameObject::onContactEnter(obj, contact);

	if (!mNetwork->isServer())
		return;

	Character* character = dynamic_cast<Character*>(obj);
	if (character)
	{
		cocos2d::Vec2 normal = contact.normal;
		if (normal.y >= -0.1f)
		{
			if (normal.y < 0.5f)
			{
				normal.y = 0.5f;
				normal.normalize();
			}

			mBody->SetLinearVelocity(b2Vec2(mBody->GetLinearVelocity().x, 0.0f));

			b2Vec2 impulse(normal.x * mImpactImpulse.x, normal.y * mImpactImpulse.y);
			mBody->ApplyLinearImpulse(impulse, mUseCenterImpulse ? mBody->GetWorldCenter() : cocos2d2Box2D(contact.point), true);
		}
	}
}

void Ball::update(float deltaTime)
{
	GameObject::update(deltaTime);

	if (mGame->mNetwork->isClient())
	{
		interpolate(deltaTime);
	}

	if (mGame->mDebugDrawer)
	{
		if (!mTransformInterpolation.isEmpty())
		{
			std::uint32_t index = mTransformInterpolation.prev(mTransformInterpolation.mHead);
			cocos2d::Vec2 lastPos;
			while (index != mTransformInterpolation.mTail)
			{
				cocos2d::Vec2 pos = mTransformInterpolation.mTransforms[index].pos;
				mGame->mDebugDrawer->drawPoint(Physics::PPM * pos, 5.f, cocos2d::Color4F::YELLOW);
				if (index != mTransformInterpolation.prev(mTransformInterpolation.mHead))
				{
					mGame->mDebugDrawer->drawLine(Physics::PPM * lastPos, Physics::PPM * pos, cocos2d::Color4F::YELLOW);
				}

				lastPos = pos;
				index = mTransformInterpolation.prev(index);
			}
		}
	}
}

void Ball::interpolate(float deltaTime)
{
	cocos2d::Vec2 position = getPosition();
	float rotation = getRotation();

	if (mTransformInterpolation.interpolate(mGame->mNetwork->mInterpolationTime, &position, 0, &rotation))
	{
		if (position.distanceSquared(getPosition()) >= 0.001f * 0.001f)
		{
			cocos2d::Vec2 vel = (position - getPosition()) / deltaTime;
			mBody->SetLinearVelocity(cocos2d2Box2D(vel));
		}
		else
			mBody->SetLinearVelocity(b2Vec2_zero);

		mNode->setRotation(rotation);
	}
	else
	{
		setPosition(position);
		setRotation(rotation);
		mBody->SetLinearVelocity(b2Vec2_zero);
	}

	BallState nextState = mCurrentState;
	mBallState.getState(mGame->mNetwork->mInterpolationTime, nextState);
	if (mCurrentState.isVisible != nextState.isVisible)
	{
		mNode->setVisible(nextState.isVisible);
	}
	if (mCurrentState.enableCharacterCollision != nextState.enableCharacterCollision)
	{
		enableCharacterCollision(nextState.enableCharacterCollision);
	}
	mCurrentState = nextState;
}

std::uint32_t Ball::calcNetPriority(std::uint16_t clientId)
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

bool Ball::serialize(NetMessage& msg)
{
	GameObject::serialize(msg);

	switch (msg.mMsgId)
	{
	case NetMessage::MSG_CreateObject:
		{
			float scaleX = mNode->getScaleX();
			float scaleY = mNode->getScaleY();

			std::string filename;
			if (!msg.isReading())
			{
				cocos2d::Sprite* sprite = dynamic_cast<cocos2d::Sprite*>(mNode.get());
				if (sprite)
				{
					filename = sprite->getResourceName();
				}
			}
			msg.serializeString(filename);

			msg.serializeFloat(scaleX);
			msg.serializeFloat(scaleY);

			msg.serializeFloat(mBodyDef.angularDamping);
			msg.serializeFloat(mBodyDef.linearDamping);

			msg.serializeFloat(mFixtureDef.density);
			msg.serializeFloat(mFixtureDef.restitution);

			msg.serializeFloat(mBallShape.m_radius);
			msg.serializeFloat(mBallShape.m_p.x);
			msg.serializeFloat(mBallShape.m_p.y);
			msg.serializeFloat(mMaxVelocity);

			msg.serializeBool(mIsKinematicOnClient);

			cocos2d::Vec2 pos = getPosition();

			if (msg.isReading())
			{
				cocos2d::Sprite* sprite = dynamic_cast<cocos2d::Sprite*>(mNode.get());
				if (sprite && !filename.empty())
				{
					sprite->initWithFile(filename);
				}
				mNode->setScaleX(scaleX);
				mNode->setScaleY(scaleY);

				createBody();
			}

			break;
		}
	case NetMessage::MSG_Sync:
		{
			BallState nextState = mCurrentState;
			cocos2d::Vec2 pos = getPosition();
			float rotation = getRotation();
			nextState.isVisible = mNode->isVisible();

			if (msg.isReading())
			{
				if (!mTransformInterpolation.isEmpty())
				{
					const TransformInterpolation::Transform& trans = mTransformInterpolation.newest();
					pos = trans.pos;
					rotation = trans.rot;
				}

				if (!mBallState.empty())
					nextState = mBallState.newest();
			}

			msg.serializeFloat(pos.x, 100);
			msg.serializeFloat(pos.y, 100);
			msg.serializeFloatShort(rotation, 1);
			msg.serializeBool(nextState.isVisible);
			msg.serializeBool(nextState.enableCharacterCollision);

			if (msg.isReading())
			{
				mTransformInterpolation.add(mGame->mNetwork->mSnapServerTime, pos, cocos2d::Vec2(), rotation);
				mBallState.add(mGame->mNetwork->mSnapServerTime, nextState);
			}

			break;
		}
	}

	return true;
}

#include "GameObjectFactory.h"
REGISTER_GAMEOBJECT(Ball, GameObject::OT_Ball, "ball", true);