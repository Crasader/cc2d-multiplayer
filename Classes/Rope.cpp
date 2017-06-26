#include "Rope.h"

#include "GameSystem.h"
#include "Physics.h"
#include "Player.h"
#include "LevelScene.h"

#include "DynamicCrayonRenderer.h"

Rope::Rope(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs)
	: GameObject(game)
{
	mCrayonRenderer = std::make_unique<DynamicCrayonRenderer>();
	mCrayonRenderer->setStyle("pencil_line_black.png", 0.5f);
	mGame->addNode(mCrayonRenderer->getNode(), 4);
}

Rope::~Rope()
{
	destroyRope();
}

void Rope::createRope(GameObject* attachedObject1, GameObject* attachedObject2, const cocos2d::Vec2& startPos, const cocos2d::Vec2& endPos)
{
	mAttachedObject1 = attachedObject1;
	mAttachedObject2 = attachedObject2;

	float length = startPos.distance(endPos);
	int segments = (length / 0.5) + 1;

	cocos2d::Vec2 segment = (endPos - startPos) / segments;
	float segmentLength = segment.length();

	b2Vec2 velocity = attachedObject1->mBody->GetLinearVelocity();

	b2Body* lastBody = attachedObject1->mBody;

	cocos2d::Vec2 segPos = startPos + segment;
	for (int i = 0; i < segments; ++i)
	{
		b2RopeJointDef jointDef = b2RopeJointDef();
		jointDef.localAnchorA = b2Vec2(0.0, 0.0);
		jointDef.localAnchorB = b2Vec2(0.0, 0.0);

		b2Body* body = nullptr;

		int endSegment = segments;
		if (mAttachedObject2)
			endSegment = segments - 1;

		if (i < endSegment)
		{
			b2BodyDef bodydef;
			if (!mAttachedObject2 && i == endSegment - 1)
				bodydef.type = b2BodyType::b2_staticBody;
			else
				bodydef.type = b2BodyType::b2_dynamicBody;
			bodydef.position.Set(segPos.x, segPos.y);
			bodydef.fixedRotation = true;
			bodydef.linearDamping = 0;

			segPos += segment;

			body = mGame->mPhysics->world->CreateBody(&bodydef);

			b2CircleShape poly;
			poly.m_radius = 0.1f;

			b2FixtureDef fixDef;
			fixDef.shape = &poly;
			fixDef.friction = 0;
			fixDef.density = 0;

			fixDef.filter.categoryBits = CollisionGroup::ROPE;
			fixDef.filter.maskBits = CollisionGroup::ALL & ~(CollisionGroup::CHARACTER | CollisionGroup::ROPE);
			body->CreateFixture(&fixDef);

			jointDef.bodyB = body;

			mRopeParts.push_back(body);

			body->SetLinearVelocity(velocity);
			velocity *= 0.5;
		}
		else
		{
			b2Vec2 relEnd = mAttachedObject2->mBody->GetLocalPoint(b2Vec2(endPos.x, endPos.y));

			jointDef.bodyB = attachedObject2->mBody;
			jointDef.localAnchorB = relEnd;
			jointDef.collideConnected = true;
		}

		jointDef.bodyA = lastBody;
		jointDef.maxLength = segmentLength;
		mJoints.push_back(mGame->mPhysics->world->CreateJoint(&jointDef));

		mCrayonRenderer->drawSegment(Physics::PPM * Box2D2cocos2(mJoints.back()->GetAnchorA()), Physics::PPM * Box2D2cocos2(mJoints.back()->GetAnchorB()));

		lastBody = body;
	}
}

void Rope::destroyRope()
{
	auto iter = mRopeParts.begin();
	auto end = mRopeParts.end();
	while (iter != end)
	{
		mGame->mPhysics->world->DestroyBody(*iter);
		++iter;
	}

	mJoints.clear();

	mAttachedObject1 = nullptr;
	mAttachedObject2 = nullptr;

	mGame->mLevelScene->mObjectLayer->removeChild(mCrayonRenderer->getNode());
	mCrayonRenderer->clear();
}

void Rope::update(float deltaTime)
{
	for (size_t i = 0; i < mJoints.size(); ++i)
	{
		cocos2d::Vec2 lastPos = Physics::PPM * Box2D2cocos2(mJoints[i]->GetAnchorA());
		cocos2d::Vec2 currPos = Physics::PPM * Box2D2cocos2(mJoints[i]->GetAnchorB());

		mCrayonRenderer->updateSegment(i, lastPos, currPos);
	}
}

#include "GameObjectFactory.h"
REGISTER_GAMEOBJECT(Rope, GameObject::OT_Rope, "rope", false);