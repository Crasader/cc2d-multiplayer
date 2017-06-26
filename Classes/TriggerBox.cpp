#include "TriggerBox.h"

#include "GameSystem.h"
#include "Physics.h"
#include "NetworkManager.h"

TriggerBox::TriggerBox(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs)
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

TriggerBox::~TriggerBox()
{

}

#include "GameObjectFactory.h"
REGISTER_GAMEOBJECT(TriggerBox, GameObject::OT_TriggerBox, "triggerBox", false);


#include "EditorObjects.h"
#include "Editor.h"

class TriggerBoxEditObject : public EditObject
{
public:
	TriggerBoxEditObject(Editor* editor, const std::string& className)
		: EditObject(editor, className)
	{
		float cellSize = (64.f);

		mNode = cocos2d::Node::create();
		mNode->setContentSize(cocos2d::Size(cellSize, cellSize));
		mNode->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
	}

	void draw()
	{
		cocos2d::Rect rect = mNode->getBoundingBox();
		mEditor->mDrawNode->drawRect(rect.origin, rect.origin + rect.size, cocos2d::Color4F::GREEN);
	}
};

EditObjectFactory<TriggerBoxEditObject> gSoccerGoalFactory("TriggerBox", "triggerBox", 5);