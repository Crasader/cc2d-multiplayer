#include "Player.h"

#include "GameSystem.h"
#include "Input.h"
#include "NetMessage.h"
#include "NetworkManager.h"
#include "Physics.h"
#include "LevelScene.h"

#include "Lobby.h"

#include "GameMode.h"

#include "Rope.h"

#include "cocos2d.h"

Player::Player(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs)
	: Character(game, pos, spawnArgs)
	, mClientId(NetworkManager::InvalidClientId)
{
	auto sprite = cocos2d::Sprite::create("CharStand.png");
	sprite->setPosition(pos * Physics::PPM);
	sprite->getTexture()->setAliasTexParameters();
	mGame->addNode(sprite, 5.0f);
	mNode = sprite;
	mNode->setScale(0.75f);

	mStandAnimation = cocos2d::Animation::create();
	mStandAnimation->retain();
	mStandAnimation->addSpriteFrameWithFile("CharStand.png");
	mStandAnimation->setDelayPerUnit(1.5f);
	mStandAnimation->setRestoreOriginalFrame(true);

	mWalkAnimation = cocos2d::Animation::create();
	mWalkAnimation->retain();
	mWalkAnimation->addSpriteFrameWithFile("CharWalk1.png");
	mWalkAnimation->addSpriteFrameWithFile("CharWalk2.png");
	mWalkAnimation->addSpriteFrameWithFile("CharWalk3.png");
	mWalkAnimation->addSpriteFrameWithFile("CharWalk4.png");
	mWalkAnimation->setDelayPerUnit(0.5f / 4.0f);
	mWalkAnimation->setRestoreOriginalFrame(true);

	mFallAnimation = cocos2d::Animation::create();
	mFallAnimation->retain();
	mFallAnimation->addSpriteFrameWithFile("CharJump.png");
	mFallAnimation->setDelayPerUnit(1.5f);
	mFallAnimation->setRestoreOriginalFrame(true);

	changeAnimation(mFallAnimation.get());

	createBody(0.5f, 1.5f);

	mClientId = spawnArgs.getInt("clientId", NetworkManager::InvalidClientId);

	mPlayerNameLabel = cocos2d::Label::createWithTTF("unknown", "fonts/Marker Felt.ttf", 20.0f);
	mPlayerNameLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
	mPlayerNameLabel->setColor(cocos2d::Color3B::GRAY);
	mPlayerNameLabel->setPosition(0.0f, 100.0f);
	mGame->addNode(mPlayerNameLabel, 10);

	if (mGame->mNetwork->isServer())
	{
		if (isLocalPlayer())
		{
			initLocalPlayer();
		}
		else
		{
			//mBody->SetType(b2BodyType::b2_kinematicBody);
		}

		Lobby::LobbyUser* lobbyUser = mGame->mLobby->getUserByClientId(mClientId);
		if (lobbyUser)
		{
			mPlayerNameLabel->setString(lobbyUser->name);
		}

		NetworkManager::PlayerController* pc = mGame->mNetwork->getPlayerController(mClientId);
		if (pc)
		{
			mTeam = pc->team;
			if (pc->team == 0)
				mPlayerNameLabel->setColor(cocos2d::Color3B::RED);
			else if (pc->team == 1)
				mPlayerNameLabel->setColor(cocos2d::Color3B::BLUE);
		}
	}

	mGame->mGameMode->onSpawnPlayer(this);
}

Player::~Player()
{
	mGame->mInput->unbindAll(this);

	detachRope();

	mGame->removePlayer(mClientId);

	if (mPlayerNameLabel)
	{
		mGame->mLevelScene->mObjectLayer->removeChild(mPlayerNameLabel);
	}

	if (mGame->mMyPlayer == this)
	{
		mGame->setMyPlayer(nullptr);
	}
}

void Player::initLocalPlayer()
{
	mGame->setMyPlayer(this);
	mGame->setCameraTarget(mNode);

	mGame->spectating(mClientId, nullptr);

	initInput();
}

void Player::initInput()
{
	mGame->mInput->unbindAll(this);
	mGame->mInput->bindAction("jump", IE_Pressed, std::bind(&Character::jump, this), this);
	mGame->mInput->bindAction("rope", IE_Pressed, std::bind(&Player::rope, this), this);
}

bool Player::isLocalPlayer() const
{
	return (mGame->mNetwork && mClientId == mGame->mNetwork->mClientId);
}

std::uint32_t Player::getGameTime()
{
	std::uint32_t currTime = mGame->mNetwork->mInterpolationTime;
	if (mGame->mNetwork->isServer())
	{
		NetworkManager::Connection* client = mGame->mNetwork->getConnection(mClientId);
		if (client)
		{
			currTime = client->gameTime;
		}
	}
	return currTime;
}

void Player::setPosition(const cocos2d::Vec2& pos)
{
	if (mBody)
	{
		float rot = getRotation();
		mBody->SetTransform(b2Vec2(pos.x, pos.y), rot);
	}
	else if (mNode)
	{
		mNode->setPosition(pos * Physics::PPM);
	}
}

void Player::kill()
{
	Character::kill();

	if (mGame->mNetwork->isServer())
	{
		mGame->spectating(mClientId);
	}
}

void Player::jump()
{
	if (mOnJumpGround && mCurrCharacterState.moveState == MS_Walking || mRope)
	{
		mCurrCharacterState.moveState = MS_Falling;
		mBody->ApplyLinearImpulse(b2Vec2(0, mRope ? mJumpRopeForce : mJumpForce), mBody->GetWorldCenter(), true);

		mJumpTimer = 0.f;
		mDoubleJumped = false;
	}
	else if (mCurrCharacterState.moveState == MS_Falling && mJumpTimer > 0.1f && !mDoubleJumped)
	{
		mDoubleJumped = true;

		mBody->SetLinearVelocity(b2Vec2(mBody->GetLinearVelocity().x, 0.f));
		mBody->ApplyLinearImpulse(b2Vec2(0, mJumpForce), mBody->GetWorldCenter(), true);
	}

	detachRope();
}

cocos2d::Vec2 getClosestPointOnLineSegment(const cocos2d::Vec2& linePointStart, const cocos2d::Vec2& linePointEnd,
	const cocos2d::Vec2& testPoint)
{
	const cocos2d::Vec2 lineDiffVect = linePointEnd - linePointStart;
	const float lineSegSqrLength = lineDiffVect.lengthSquared();

	const cocos2d::Vec2 lineToPointVect = testPoint - linePointStart;
	const float dotProduct = lineDiffVect.dot(lineToPointVect);

	const float percAlongLine = dotProduct / lineSegSqrLength;

	if (percAlongLine < 0.0f)
		return linePointStart;
	if (percAlongLine  > 1.0f)
		return linePointEnd;
	return (linePointStart + (percAlongLine * (linePointEnd - linePointStart)));
}

struct RopeTest
{
	cocos2d::Vec2 target;
	GameObject* obj;
};

void Player::rope()
{
	if (!mRope)
	{
		std::vector<RopeTest> tests;

		cocos2d::Vec2 pos = getPosition();

		float32 radius = 5.f;
		b2Vec2 center(pos.x + (mCurrCharacterState.isLookRight ? radius : -radius), pos.y + radius);
		
		b2AABB aabb;
		aabb.lowerBound = center - b2Vec2(radius, radius);
		aabb.upperBound = center + b2Vec2(radius, radius);

		mGame->addDebugRect(cocos2d::Vec2(aabb.lowerBound.x, aabb.lowerBound.y),
			cocos2d::Vec2(aabb.upperBound.x, aabb.upperBound.y),
			cocos2d::Color4F::RED, 2.0f);

		QueryAllCallback aabbcb(center);
		mGame->mPhysics->world->QueryAABB(&aabbcb, aabb);
		for (size_t i = 0; i < aabbcb.m_fixtures.size(); ++i)
		{
			b2Body* body = aabbcb.m_fixtures[i]->GetBody();
			b2Vec2 bodyCom = body->GetWorldCenter();
			GameObject * gameObject = static_cast<GameObject*>(aabbcb.m_fixtures[i]->GetUserData());

			if (!gameObject)
				continue;

			if (dynamic_cast<Character*>(gameObject) || dynamic_cast<Rope*>(gameObject))
				continue;

			auto pos = getPosition() + cocos2d::Vec2(mCurrCharacterState.isLookRight ? 0.5f : -0.5f, 0.f);//0.2

			const b2AABB childAABB = aabbcb.m_fixtures[i]->GetAABB(aabbcb.mChildIndex[i]);
			cocos2d::Vec2 pA(childAABB.lowerBound.x + 1.f, childAABB.upperBound.y);
			cocos2d::Vec2 pB(childAABB.upperBound.x - 1.f, childAABB.upperBound.y);
			cocos2d::Vec2 ve = getClosestPointOnLineSegment(pA, pB, cocos2d::Vec2(center.x, center.y));

			ve.x = cocos2d::clampf(ve.x, (aabb.lowerBound.x + 1.0f), (aabb.upperBound.x - 1.0f));
			ve.y = cocos2d::clampf(ve.y, aabb.lowerBound.y, aabb.upperBound.y);
			auto target = b2Vec2(ve.x, ve.y);

			auto dir = target - b2Vec2(pos.x, pos.y);
			dir.Normalize();

			mGame->addDebugRect(Box2D2cocos2(childAABB.lowerBound), Box2D2cocos2(childAABB.upperBound), cocos2d::Color4F::RED, 2.0f);
			mGame->addDebugLine(cocos2d::Vec2(pos.x, pos.y), ve, cocos2d::Color4F::RED, 2.0f);
			
			RayCastCallback cb;
			mGame->mPhysics->world->RayCast(&cb, b2Vec2(pos.x, pos.y), target);
			if (cb.m_fixture == aabbcb.m_fixtures[i])
			{
				cocos2d::Vec2 realTarget;
				if (cb.m_fixture)
					realTarget = cocos2d::Vec2(cb.m_point.x, cb.m_point.y);
				else
					realTarget = cocos2d::Vec2(target.x, target.y);

				RopeTest test;
				test.target = realTarget;
				test.obj = gameObject;
				tests.push_back(test);
			}
		}

		auto handPos = getPosition() + cocos2d::Vec2(mCurrCharacterState.isLookRight ? 0.2 : -0.2, 0.5);

		RopeTest* result = nullptr;
		if (tests.size() > 0)
		{
			result = &tests[0];
			float minSqDist = 999999.f;
			for (size_t i = 1; i < tests.size(); ++i)
			{
				const float sqDist = pos.distanceSquared(tests[i].target);
				if (minSqDist > sqDist)
				{
					minSqDist = sqDist;
					result = &tests[i];
				}
			}
		}

		if (result)
		{
			cocos2d::Vec2 startPos = getPosition();

			Parameters spawnArgs;
			mRope = dynamic_cast<Rope*>(mGame->spawnGameObject("rope", cocos2d::Vec2(), spawnArgs));
			if (mRope)
			{
				mRope->createRope(this, result->obj, startPos, result->target);

				NetMessage msg;
				msg.writeUInt(getGameTime());

				msg.writeUInt(mRope->mAttachedObject2->mNetId);

				msg.writeFloat(startPos.x);
				msg.writeFloat(startPos.y);

				msg.writeFloat(result->target.x);
				msg.writeFloat(result->target.y);

				sendNetEvent(NE_AttachRope, msg);

				mGame->addDebugLine(cocos2d::Vec2(pos.x, pos.y), cocos2d::Vec2(result->target.x, result->target.y),
					cocos2d::Color4F::GREEN, 2.0f);
			}
		}
	}
	else
	{
		detachRope();
	}
}

void Player::detachRope()
{
	if (!mRope)
		return;

	mRope->destroyMe();
	mRope = nullptr;

	if (isLocalPlayer())
	{
		sendNetEvent(NE_DetachRope);
	}
}

void Player::update(float deltaTime)
{
	if (!isLocalPlayer())
	{
		//if (mGame->mNetwork->isClient())
		{
			interpolate(deltaTime);
		}
	}

	if (isLocalPlayer())
	{
		mCurrCharacterState.moveSpeed = std::abs(mBody->GetLinearVelocity().x) - (mGoundBody ? std::abs(mGoundBody->GetLinearVelocity().x) : 0.f);

		mCurrCharacterState.moveVec = cocos2d::Vec2::ZERO;

		float rightValue = mGame->mInput->getAxisValue("right");
		mCurrCharacterState.moveVec.x = fabsf(rightValue) > 0.5f ? rightValue : 0.0f;

		if (mGame->mInput->getAxisValue("run") > 0.8f)
			mCurrPlayerState.isRunning = true;
		else
			mCurrPlayerState.isRunning = false;
	}

	if (mCurrPlayerState.isRunning && mCurrCharacterState.moveState == MS_Walking)
		mMaxSpeed = mRunSpeed;
	else
		mMaxSpeed = mWalkSpeed;

	if (!mCurrCharacterState.moveVec.isZero())
	{
		if (mCurrCharacterState.moveState == MS_Walking)
		{
			mAcceleration = mNormalAcceleration;
		}
		if (mRope)
		{
			mMaxSpeed = mRopeSpeed;
			if (mBody->GetLinearVelocity().y <= 0.f)
				mAcceleration = mRopeAcceleration;
			else
				mAcceleration = 0.f;
		}
		else if (mCurrCharacterState.moveState == MS_Falling)
		{
			mAcceleration = mAirAcceleration;
		}
	}

	MoveState saveState = (MoveState)mCurrCharacterState.moveState;

	Character::update(deltaTime);

	if (!isLocalPlayer())
	{
		mCurrCharacterState.moveState = saveState;
	}

	if (mCurrCharacterState.moveState == MS_Falling)
	{
		mJumpTimer += deltaTime;

		changeAnimation(mFallAnimation.get());
	}
	else
	{
		if (getMovingSpeed() < 0.1f)
		{
			changeAnimation(mStandAnimation.get());
		}
		else
		{
			changeAnimation(mWalkAnimation.get());
			if (mSpeedAction.get())
			{
				float scale = (getMovingSpeed() / mWalkSpeed);
				scale = std::max(scale, 0.5f);
				mSpeedAction->setSpeed(scale);
			}
		}
	}

	if (mPlayerNameLabel)
		mPlayerNameLabel->setPosition((getPosition() + cocos2d::Vec2(0.f, 1.8f)) * Physics::PPM);

	if (mGame->mDebugDrawer)
	{
		// Draw Network relevant distance
		mGame->mDebugDrawer->drawCircle(Physics::PPM * getPosition(), Physics::PPM * mNetwork->getRelevantDistance(), 0, 32, false, cocos2d::Color4F::RED);
		mGame->mDebugDrawer->drawCircle(Physics::PPM * getPosition(), Physics::PPM * mNetwork->getRelevantDistance() * 2.f, 0, 32, false, cocos2d::Color4F::BLUE);


		if (!isLocalPlayer() && !mTransformInterpolation.isEmpty())
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

	static cocos2d::DrawNode* dno = nullptr;
	if (!dno)
	{
		dno = cocos2d::DrawNode::create();
		mGame->mLevelScene->mObjectLayer->addChild(dno, 20);
	}
	else
	{
		dno->clear();
		dno->drawCircle(Physics::PPM * getPosition(), Physics::PPM * mNetwork->getRelevantDistance(), 0, 32, false, cocos2d::Color4F::RED);
		dno->drawCircle(Physics::PPM * getPosition(), Physics::PPM * mNetwork->getRelevantDistance() * 2.f, 0, 32, false, cocos2d::Color4F::BLUE);
	}
}

void Player::interpolate(float deltaTime)
{
	if (mPlayerState.empty())
		mPlayerState.init(10);
	if (mTransformInterpolation.isEmpty())
		mTransformInterpolation.init(10);
	if (mCharacterState.empty())
		mCharacterState.init(10);

	cocos2d::Vec2 position = getPosition();
	cocos2d::Vec2 vel = Box2D2cocos2(mBody->GetLinearVelocity());
	float rotation = getRotation();

	std::uint32_t currTime = mGame->mNetwork->getInterpolationTime();
	NetworkManager::Connection* client = mGame->mNetwork->getConnection(mClientId);
	if (client)
	{
		currTime = client->gameTime;
	}

	if (mTransformInterpolation.interpolate(currTime, &position, &vel, 0))
	{
		if (position.distanceSquared(getPosition()) >= 0.001f * 0.001f)
		{
			cocos2d::Vec2 vel2 = (position - getPosition()) / deltaTime;
			mBody->SetLinearVelocity(cocos2d2Box2D(vel2));
		}
		else
			mBody->SetLinearVelocity(b2Vec2_zero);
	}
	else
	{
		setPosition(position);
		mBody->SetLinearVelocity(b2Vec2(vel.x, vel.y));
	}

	CharacterState characterState = mCurrCharacterState;
	mCharacterState.getState(currTime, characterState);

	changeDirection(characterState.isLookRight);
	mCurrCharacterState = characterState;

	mPlayerState.getState(currTime, mCurrPlayerState);
}

void Player::onNetEvent(std::uint8_t eventId)
{
	switch (eventId)
	{
	case NE_DetachRope:
		detachRope();

		if (mGame->mNetwork->isServer())
		{
			sendNetEvent(NE_DetachRope);
		}
		break;
	}
}

void Player::onNetEvent(std::uint8_t eventId, NetMessage& msg)
{
	switch (eventId)
	{
	case NE_AttachRope:
	{
		if (isLocalPlayer())
			return;

		std::uint32_t time = 0;
		std::uint32_t netId = NetworkManager::InvalidNetId;
		cocos2d::Vec2 startPos;
		cocos2d::Vec2 endPos;

		msg.readUInt(time);
		msg.readUInt(netId);
		msg.readFloat(startPos.x);
		msg.readFloat(startPos.y);
		msg.readFloat(endPos.x);
		msg.readFloat(endPos.y);

		if (mGame->mNetwork->isServer())
		{
			NetMessage newMsg;
			newMsg.writeUInt(time);
			newMsg.writeUInt(netId);
			newMsg.writeFloat(startPos.x);
			newMsg.writeFloat(startPos.y);
			newMsg.writeFloat(endPos.x);
			newMsg.writeFloat(endPos.y);
			sendNetEvent(NE_AttachRope, newMsg);
		}

		detachRope();

		GameObject* obj = dynamic_cast<GameObject*>(mGame->mNetwork->getObject(netId));

		Parameters spawnArgs;
		mRope = dynamic_cast<Rope*>(mGame->spawnGameObject("rope", cocos2d::Vec2(), spawnArgs));
		if (mRope)
		{
			mRope->createRope(this, obj, startPos, endPos);
		}
		break;
	}
	}
}

bool Player::serialize(NetMessage& msg)
{
	if (msg.isReading() && msg.mMsgId == NetMessage::MSG_Sync && isLocalPlayer())
		return false;
	if (!msg.isReading() && msg.mMsgId == NetMessage::MSG_Sync && msg.mClientId == mClientId && mGame->mNetwork->isServer())
		return false;

	if (!Character::serialize(msg))
	{
		return false;
	}

	if (msg.mMsgId == NetMessage::MSG_CreateObject)
	{
		msg.serializeUShort(mClientId);
		msg.serializeInt(mTeam);

		if (msg.isReading())
		{
			if (mGame->mNetwork->getClientId() != NetworkManager::InvalidClientId &&
				mGame->mNetwork->getClientId() == mClientId)
			{
				mBody->SetType(b2BodyType::b2_dynamicBody);
				initLocalPlayer();
			}
			else
			{
				mBody->SetType(b2BodyType::b2_kinematicBody);
			}

			Lobby::LobbyUser* lobbyUser = mGame->mLobby->getUserByClientId(mClientId);
			if (lobbyUser)
			{
				mPlayerNameLabel->setString(lobbyUser->name);
			}

			if (mTeam == 0)
				mPlayerNameLabel->setColor(cocos2d::Color3B::RED);
			else if (mTeam == 1)
				mPlayerNameLabel->setColor(cocos2d::Color3B::BLUE);
		}
	}
	else if (msg.mMsgId == NetMessage::MSG_Sync)
	{
		PlayerState nextState = mCurrPlayerState;

		if (msg.isReading() && !mPlayerState.empty())
		{
			nextState = mPlayerState.newest();
		}

		msg.serializeBool(nextState.isRunning);

		if (msg.isReading())
		{
			if (mPlayerState.empty())
				mPlayerState.init(10);

			std::uint32_t currTime = mGame->mNetwork->mSnapServerTime;
			mPlayerState.add(currTime, nextState);
		}
	}

	return true;
}

#include "GameObjectFactory.h"
REGISTER_GAMEOBJECT(Player, GameObject::OT_Player, "player", true);