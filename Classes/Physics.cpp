#include "Physics.h"

#include "platform/CCPlatformMacros.h"

#include "GameObject.h"

const float Physics::PPM = 32.0f;

class MyContactListener : public b2ContactListener {

public:
	MyContactListener()
	{

	}
	~MyContactListener()
	{

	}

	virtual void BeginContact(b2Contact* contact)
	{
		GameObject* objA = static_cast<GameObject*>(contact->GetFixtureA()->GetUserData());
		GameObject* objB = static_cast<GameObject*>(contact->GetFixtureB()->GetUserData());

		b2WorldManifold worldManifold;
		contact->GetWorldManifold(&worldManifold);

		ContactInfo conInfoA;
		conInfoA.fixture = contact->GetFixtureA();
		conInfoA.otherFixture = contact->GetFixtureB();
		conInfoA.point = Box2D2cocos2(worldManifold.points[0]);
		conInfoA.normal = Box2D2cocos2(worldManifold.normal) * -1.f;
		conInfoA.impulse = 0.f;

		ContactInfo conInfoB;
		conInfoB.fixture = contact->GetFixtureB();
		conInfoB.otherFixture = contact->GetFixtureA();
		conInfoB.point = Box2D2cocos2(worldManifold.points[0]);
		conInfoB.normal = Box2D2cocos2(worldManifold.normal);
		conInfoB.impulse = 0.f;

		if (objA && !objA->mIsDestroyed)
			objA->onContactEnter(objB, conInfoA);

		if (objB && !objB->mIsDestroyed)
			objB->onContactEnter(objA, conInfoB);
	}
	virtual void EndContact(b2Contact* contact)
	{
		GameObject* objA = static_cast<GameObject*>(contact->GetFixtureA()->GetUserData());
		GameObject* objB = static_cast<GameObject*>(contact->GetFixtureB()->GetUserData());

		b2WorldManifold worldManifold;
		contact->GetWorldManifold(&worldManifold);

		ContactInfo conInfoA;
		conInfoA.fixture = contact->GetFixtureA();
		conInfoA.otherFixture = contact->GetFixtureB();
		conInfoA.point = Box2D2cocos2(worldManifold.points[0]);
		conInfoA.normal = Box2D2cocos2(worldManifold.normal) * -1.f;
		conInfoA.impulse = 0.f;

		ContactInfo conInfoB;
		conInfoB.fixture = contact->GetFixtureB();
		conInfoB.otherFixture = contact->GetFixtureA();
		conInfoB.point = Box2D2cocos2(worldManifold.points[0]);
		conInfoB.normal = Box2D2cocos2(worldManifold.normal);
		conInfoB.impulse = 0.f;

		if (objA && !objA->mIsDestroyed)
			objA->onContactLeave(objB, conInfoA);

		if (objB && !objB->mIsDestroyed)
			objB->onContactLeave(objA, conInfoB);
	}
	virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
	{
		GameObject* objA = static_cast<GameObject*>(contact->GetFixtureA()->GetUserData());
		GameObject* objB = static_cast<GameObject*>(contact->GetFixtureB()->GetUserData());

		if (objA && !objA->mIsDestroyed)
		{
			objA->onContactPreSolve(contact, oldManifold);
		}
		if (objB && !objB->mIsDestroyed)
		{
			objB->onContactPreSolve(contact, oldManifold);
		}
	}
	virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
	{
		GameObject* objA = static_cast<GameObject*>(contact->GetFixtureA()->GetUserData());
		GameObject* objB = static_cast<GameObject*>(contact->GetFixtureB()->GetUserData());

		if (objA && !objA->mIsDestroyed)
		{
			objA->onContactPostSolve(contact, impulse);
		}
		if (objB && !objB->mIsDestroyed)
		{
			objB->onContactPostSolve(contact, impulse);
		}
	}
};

Physics::Physics()
{
	world = new b2World(b2Vec2(0.0f, -10.0f));

	world->SetAllowSleeping(true);
	world->SetContinuousPhysics(true);

	mContactListener = new MyContactListener();
	world->SetContactListener(mContactListener);
}

Physics::~Physics()
{
	if (mContactListener)
	{
		delete mContactListener;
		mContactListener = 0;
	}

	CC_SAFE_DELETE(world);
}

void Physics::update(float deltaTime)
{
	int velocityIterations = 8;
	int positionIterations = 1;

	world->Step(deltaTime, velocityIterations, positionIterations);
	world->DrawDebugData();
}