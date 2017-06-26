#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include "Box2D/Box2D.h"
#include "math/Vec2.h"

#include <vector>

class MyContactListener;

inline b2Vec2 cocos2d2Box2D(const cocos2d::Vec2& v) { return b2Vec2(v.x, v.y); }
inline cocos2d::Vec2 Box2D2cocos2(const b2Vec2& v) { return cocos2d::Vec2(v.x, v.y); }

struct ContactInfo
{
	b2Fixture* fixture;
	b2Fixture* otherFixture;
	cocos2d::Vec2 point;
	cocos2d::Vec2 normal;
	float impulse;
};

enum CollisionGroup {
	NOTHING = 0,
	BOUNDS = (1 << 0),//0x0001,
	ENEMY = (1 << 1),//0x0002,
	CHARACTER = (1 << 2),//0x0004,
	ROPE = (1 << 3),//0x0008,
	BLOCK = (1 << 4),//0x0010,
	DEBRIS = (1 << 5),//0x0020,

	ALL = BOUNDS | ENEMY | CHARACTER | ROPE | BLOCK | DEBRIS
};

class RayCastCallback : public b2RayCastCallback
{
public:
	RayCastCallback()
	{
		m_fixture = NULL;
	}

	virtual float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point,
		const b2Vec2& normal, float32 fraction)
	{
		if (!checkSensor && fixture->IsSensor())
			return 1.0f;

		if (!(fixture->GetFilterData().categoryBits & filterMask))
			return 1.0f;

		m_fixture = fixture;
		m_point = point;
		m_normal = normal;

		return fraction;
	}

	b2Fixture* m_fixture;
	b2Vec2 m_point;
	b2Vec2 m_normal;

	bool checkSensor = false;
	int16 filterMask = CollisionGroup::ALL;
};

class RayCastCallbackNotMe : public RayCastCallback
{
public:
	RayCastCallbackNotMe(b2Body* me)
		: mMe(me)
	{}

	virtual float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point,
		const b2Vec2& normal, float32 fraction)
	{
		if (mMe == fixture->GetBody())
			return 1.0f;

		return RayCastCallback::ReportFixture(fixture, point,
			normal, fraction);
	}

	b2Body* mMe = nullptr;
};

class QueryCallback : public b2QueryCallback
{
public:
	QueryCallback(const b2Vec2& point)
	{
		m_point = point;
		m_fixture = NULL;
	}

	bool ReportFixture(b2Fixture* fixture)
	{
		m_fixture = fixture;
		return false;
	}

	b2Vec2 m_point;
	b2Fixture* m_fixture;
};

class QueryAllCallback : public b2QueryCallback
{
public:
	QueryAllCallback(const b2Vec2& point)
	{
		m_point = point;
	}

	virtual bool ReportFixture(b2Fixture* fixture, int32 childIndex)
	{
		if (!(!checkSensor && fixture->IsSensor()))
		{
			m_fixtures.push_back(fixture);
			mChildIndex.push_back(childIndex);
		}

		return true;
	}

	b2Vec2 m_point;
	std::vector<b2Fixture*> m_fixtures;
	std::vector<int32> mChildIndex;

	bool checkSensor = false;
};

class QueryAllCallbackInPoint : public QueryAllCallback
{
public:
	QueryAllCallbackInPoint(const b2Vec2& point)
		: QueryAllCallback(point)
	{}

	virtual bool ReportFixture(b2Fixture* fixture, int32 childIndex)
	{
		if (!fixture->TestPoint(m_point))
			return true;

		return QueryAllCallback::ReportFixture(fixture, childIndex);
	}
};

class QueryAllCallbackPoint : public b2QueryCallback
{
public:
	struct Test
	{
		Test(b2Fixture* _fixture, const b2Vec2& _hitPoint)
			: fixture(_fixture)
			, hitPoint(_hitPoint)
		{}

		b2Fixture* fixture;
		b2Vec2 hitPoint;
	};

	QueryAllCallbackPoint(const b2Vec2& point)
	{
		m_point = point;
	}

	bool ReportFixture(b2Fixture* fixture, int32 childIndex)
	{
		b2RayCastOutput out;
		b2RayCastInput in;
		in.maxFraction = 1.0f;
		in.p1 = m_point;
		in.p2 = fixture->GetBody()->GetWorldCenter();
		//int childIndex = 0;
		fixture->RayCast(&out, in, childIndex);

		b2Vec2 hitPos = (in.p2 - in.p1);
		hitPos *= out.fraction;
		hitPos = in.p1 + hitPos;

		m_fixtures.emplace_back(fixture, hitPos);

		return true;
	}

	b2Vec2 m_point;
	std::vector<Test> m_fixtures;
};

class Physics
{
public:
	static const float PPM;

public:
	Physics();
	~Physics();

	void update(float deltaTime);

	b2World* world;
	MyContactListener* mContactListener;
};

#endif