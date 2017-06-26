#include "Coin.h"

#include "GameSystem.h"
#include "LevelScene.h"
#include "Physics.h"

Coin::Coin(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs)
	: GameObject(game)
{
	auto sprite = cocos2d::Sprite::create("Coin.png");
	mNode = sprite;
	setPosition(pos);
	game->addNode(sprite, 6);

	if (sprite)
	{
		b2BodyDef bodyDef;
		bodyDef.position.Set(pos.x, pos.y);
		bodyDef.type = b2BodyType::b2_staticBody;
		mBody = mGame->mPhysics->world->CreateBody(&bodyDef);

		b2PolygonShape poly;
		float w = sprite->getContentSize().width;
		float h = sprite->getContentSize().height;
		poly.SetAsBox((sprite->getContentSize().width / Physics::PPM) / 2.0f, (sprite->getContentSize().height / Physics::PPM) / 2.0f);
		b2Fixture* fix = mBody->CreateFixture(&poly, 0.0f);
		fix->SetUserData(this);
		fix->SetSensor(true);
	}
}

Coin::~Coin()
{
}

void Coin::update(float deltaTime)
{
	GameObject::update(deltaTime);
}

#include "GameObjectFactory.h"
REGISTER_GAMEOBJECT(Coin, GameObject::OT_Coin, "coin", true);