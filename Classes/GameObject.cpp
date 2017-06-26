#include "GameObject.h"

#include "GameSystem.h"
#include "LevelScene.h"

#include "Physics.h"
#include "NetworkManager.h"
#include "ScriptManager.h"

GameObject::GameObject(GameSystem* game)
	: mGame(game)
	, mLocalIndex(NetworkManager::InvalidNetId)
{
	mGame->addGameObject(this);
}

GameObject::~GameObject()
{
	mOnDestroyed(this);

	if (mNode)
	{
		mGame->mLevelScene->mObjectLayer->removeChild(mNode);
		mNode = nullptr;
	}

	if (mBody)
	{
		mBody->GetWorld()->DestroyBody(mBody);
		mBody = nullptr;
	}
}

void GameObject::onNetDestroy()
{
	destroyMe();
}

void GameObject::destroyMe()
{
	if (mIsDestroyed)
		return;

	mGame->removeGameObject(this);
}

const cocos2d::Vec2 GameObject::getPosition() const
{
	if (mBody)
	{
		return cocos2d::Vec2(mBody->GetPosition().x, mBody->GetPosition().y);
	}
	else if (mNode)
	{
		return mNode->getPosition() / Physics::PPM;
	}
	return cocos2d::Vec2::ZERO;
}

void GameObject::setPosition(const cocos2d::Vec2& pos)
{
	if (mBody)
	{
		float rot = getRotation();
		mBody->SetTransform(b2Vec2(pos.x, pos.y), rot);
	}
	if (mNode)
	{
		mNode->setPosition(pos * Physics::PPM);
	}
}

float GameObject::getRotation() const
{
	if (mBody)
	{
		return -CC_RADIANS_TO_DEGREES(mBody->GetAngle());
	}
	else if (mNode)
	{
		return mNode->getRotation();
	}
	return 0.f;
}

void GameObject::setRotation(float degree)
{
	if (mBody)
	{
		cocos2d::Vec2 pos = getPosition();
		mBody->SetTransform(b2Vec2(pos.x, pos.y), CC_DEGREES_TO_RADIANS(-degree));
	}
	else if (mNode)
	{
		mNode->setRotation(degree);
	}
}

void GameObject::handleEvent(const EventData* evt)
{
	if (dynamic_cast<const ScriptFunctionEvent*>(evt))
	{
		const ScriptFunctionEvent* scriptEvent = static_cast<const ScriptFunctionEvent*>(evt);

		if (scriptEvent->eventName == "destroyed")
		{
			mOnDestroyed.add(mGame->mScriptManager->mGlobalScript->getOrCreateFunction(scriptEvent->funcName));
		}
		else if (scriptEvent->eventName == "contactEnter")
		{
			mOnContactEnter.add(mGame->mScriptManager->mGlobalScript->getOrCreateFunction(scriptEvent->funcName));
		}
		else if (scriptEvent->eventName == "contactLeave")
		{
			mOnContactLeave.add(mGame->mScriptManager->mGlobalScript->getOrCreateFunction(scriptEvent->funcName));
		}
	}
}

void GameObject::onContactEnter(GameObject* obj, const ContactInfo& contact)
{
	mOnContactEnter(this, obj, contact);
}

void GameObject::onContactLeave(GameObject* obj, const ContactInfo& contact)
{
	mOnContactLeave(this, obj, contact);
}

void GameObject::update(float deltaTime)
{
	if (mBody && mNode)
	{
		b2Vec2 pos = mBody->GetPosition();
		float rot = -mBody->GetAngle();
		mNode->setPosition(cocos2d::Vec2(pos.x * Physics::PPM, pos.y * Physics::PPM));
		mNode->setRotation(CC_RADIANS_TO_DEGREES(rot));
	}

	if (mBody && mMaxVelocity > 0.f)
	{
		b2Vec2 vel = mBody->GetLinearVelocity();
		if (vel.LengthSquared() > mMaxVelocity * mMaxVelocity)
		{
			vel.Normalize();
			mBody->SetLinearVelocity(b2Vec2(vel.x * mMaxVelocity, vel.y * mMaxVelocity));
		}
	}
}

void GameObject::writeNetCreateInit(NetMessage& msg)
{
	msg.writeFloat(getPosition().x);
	msg.writeFloat(getPosition().y);
}