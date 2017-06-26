#include "KillZone.h"

#include "GameSystem.h"
#include "Physics.h"
#include "NetworkManager.h"
#include "Character.h"

KillZone::KillZone(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs)
	: GameObject(game)
{
	float scaleX = spawnArgs.getFloat("sx", 1.f);
	float scaleY = spawnArgs.getFloat("sy", 1.f);

	mNode = cocos2d::Node::create();
	mNode->setPosition(pos * Physics::PPM);
	mGame->addNode(mNode, 0);

	b2BodyDef bodyDef;
	bodyDef.position.Set(pos.x, pos.y);
	bodyDef.type = b2BodyType::b2_staticBody;
	mBody = mGame->mPhysics->world->CreateBody(&bodyDef);

	b2PolygonShape poly;
	poly.SetAsBox(scaleX, scaleY);
	b2Fixture* fixture = mBody->CreateFixture(&poly, 0.0f);
	fixture->SetSensor(true);
	fixture->SetUserData(this);
}

KillZone::~KillZone()
{

}

void KillZone::onContactEnter(GameObject* object, const ContactInfo& contact)
{
	if (!object || !mGame->mNetwork->isServer())
		return;

	Character* character = dynamic_cast<Character*>(object);
	if (character)
	{
		character->kill();
	}
	else
	{
		object->onTakeDamage(this, 100.f, contact.point);
	}
}

#include "GameObjectFactory.h"
REGISTER_GAMEOBJECT(KillZone, GameObject::OT_KillZone, "killzone", false);