#include "TargetPoint.h"

#include "GameSystem.h"

TargetPoint::TargetPoint(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs)
	: GameObject(game)
{
	mNode = cocos2d::Node::create();
	setPosition(pos);
	mGame->addNode(mNode, 5);
}

TargetPoint::~TargetPoint()
{
}

#include "GameObjectFactory.h"
REGISTER_GAMEOBJECT(TargetPoint, GameObject::OT_TargetPoint, "targetPoint", false);


#include "EditorObjects.h"
#include "Editor.h"

class TargetPointEditObject : public EditObject
{
public:
	TargetPointEditObject(Editor* editor, const std::string& className)
		: EditObject(editor, className)
	{
		mNode = cocos2d::Node::create();
		mNode->setContentSize(cocos2d::Size(20, 20));
		mNode->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
	}

	void draw()
	{
		mEditor->mDrawNode->drawLine(mNode->getPosition() - cocos2d::Vec2(10.f, 0.f), mNode->getPosition() + cocos2d::Vec2(10.f, 0.f), cocos2d::Color4F::GREEN);
		mEditor->mDrawNode->drawLine(mNode->getPosition() - cocos2d::Vec2(0.f, 10.f), mNode->getPosition() + cocos2d::Vec2(0.f, 10.f), cocos2d::Color4F::GREEN);
	}

	bool canScale() override { return false; }

};
EditObjectFactory<TargetPointEditObject> gTargetPointFactory("TargetPoint", "targetPoint", 5);