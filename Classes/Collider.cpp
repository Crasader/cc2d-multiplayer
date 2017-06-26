#include "Collider.h"

#include "GameSystem.h"
#include "Physics.h"

Collider::Collider(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs)
	: GameObject(game)
{
	float width = 1.f;
	float height = 1.f;

	spawnArgs.getFloat("sx", width, 1.f);
	spawnArgs.getFloat("sy", height, 1.f);

	b2BodyDef bodyDef;
	bodyDef.position.Set(pos.x, pos.y);
	bodyDef.type = b2BodyType::b2_staticBody;
	mBody = mGame->mPhysics->world->CreateBody(&bodyDef);

	b2Filter filter;
	filter.categoryBits = CollisionGroup::BOUNDS;
	filter.maskBits = CollisionGroup::ALL;

	b2PolygonShape polyShape;
	polyShape.SetAsBox(width, height);

	b2Fixture* fixture = mBody->CreateFixture(&polyShape, 0.0f);
	fixture->SetFriction(1.0f);
	fixture->SetFilterData(filter);
	fixture->SetUserData(this);
}

Collider::~Collider()
{

}

#include "GameObjectFactory.h"
REGISTER_GAMEOBJECT(Collider, GameObject::OT_Collider, "collider", false);

ChainCollider::ChainCollider(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs)
	: GameObject(game)
{
	float width = 1.f;
	float height = 1.f;

	if (spawnArgs.hasProp("points"))
	{
		b2BodyDef bodyDef;
		bodyDef.position.Set(pos.x, pos.y);
		bodyDef.type = b2BodyType::b2_staticBody;
		mBody = mGame->mPhysics->world->CreateBody(&bodyDef);

		b2Filter filter;
		filter.categoryBits = CollisionGroup::BOUNDS;
		filter.maskBits = CollisionGroup::ALL;

		b2ChainShape chainShape;
		
		std::vector<b2Vec2> points;
		std::vector<Parameters> arr;
		spawnArgs.getArray("points", arr);
		for (size_t i = 0; i < arr.size(); ++i)
		{
			float x = arr[i].getFloat("x");
			float y = arr[i].getFloat("y");
			points.emplace_back(x, y);
		}
		chainShape.CreateChain(&points[0], points.size());

		b2Fixture* groundFix = mBody->CreateFixture(&chainShape, 0.0f);
		groundFix->SetFriction(0.2f);
		groundFix->SetFilterData(filter);
		groundFix->SetUserData(this);
	}
}

ChainCollider::~ChainCollider()
{

}

REGISTER_GAMEOBJECT(ChainCollider, GameObject::OT_ChainCollider, "chainCollider", false);