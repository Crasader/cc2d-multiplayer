#include "CrayonBox.h"

#include "GameSystem.h"
#include "Physics.h"
#include "LevelScene.h"

#include "DynamicCrayonRenderer.h"

CrayonBox::CrayonBox(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs)
	: GameObject(game)
{
	float width = spawnArgs.getFloat("sx", 1.f);
	float height = spawnArgs.getFloat("sy", 1.f);

	cocos2d::Vec2 origin;

	mCrayonRenderer = std::make_unique<DynamicCrayonRenderer>();
	mCrayonRenderer->drawSegment(cocos2d::Vec2(origin.x - 32.f * width, origin.y - 32.f * height), cocos2d::Vec2(origin.x - 32.f * width, origin.y + 32.f * height));
	mCrayonRenderer->drawSegment(cocos2d::Vec2(origin.x - 32.f * width, origin.y + 32.f * height), cocos2d::Vec2(origin.x + 32.f * width, origin.y + 32.f * height));
	mCrayonRenderer->drawSegment(cocos2d::Vec2(origin.x + 32.f * width, origin.y + 32.f * height), cocos2d::Vec2(origin.x + 32.f * width, origin.y - 32.f * height));
	mCrayonRenderer->drawSegment(cocos2d::Vec2(origin.x + 32.f * width, origin.y - 32.f * height), cocos2d::Vec2(origin.x - 32.f * width, origin.y - 32.f * height));

	mNode = mCrayonRenderer->getNode();
	mNode->setPosition(pos * Physics::PPM);
	mGame->addNode(mNode, 3);

	b2BodyDef bodyDef;
	bodyDef.position.Set(pos.x, pos.y);
	bodyDef.type = b2BodyType::b2_staticBody;
	mBody = mGame->mPhysics->world->CreateBody(&bodyDef);

	b2Filter filter;
	filter.categoryBits = CollisionGroup::BOUNDS;
	filter.maskBits = CollisionGroup::ALL;

	b2PolygonShape shape;
	shape.SetAsBox(width, height);

	b2FixtureDef fd;
	fd.shape = &shape;
	fd.density = 0.0f;
	fd.friction = 0.6f;
	fd.filter = filter;
	fd.userData = this;
	mBody->CreateFixture(&fd);
}

CrayonBox::~CrayonBox()
{

}

#include "GameObjectFactory.h"
REGISTER_GAMEOBJECT(CrayonBox, GameObject::OT_CrayonBox, "crayonBox", false);


#include "EditorObjects.h"
#include "Editor.h"

class CrayonBoxEditObject : public EditObject
{
public:
	CrayonBoxEditObject(Editor* editor, const std::string& className)
		: EditObject(editor, className)
	{
		mCrayonRenderer = std::make_unique<DynamicCrayonRenderer>();
		mNode = mCrayonRenderer->mNode;
		mNode->setContentSize(cocos2d::Size(64.f, 64.f));
	}

	void draw()
	{
		cocos2d::Rect rect = mNode->getBoundingBox();
		mEditor->mDrawNode->drawRect(rect.origin, rect.origin + rect.size, cocos2d::Color4F::BLUE);

		float width = getScaleX();
		float height = getScaleY();
		cocos2d::Vec2 origin = cocos2d::Vec2(rect.size / 2.f);

		mCrayonRenderer->clear();
		mCrayonRenderer->drawSegment(cocos2d::Vec2(origin.x - 32.f * width, origin.y - 32.f * height), cocos2d::Vec2(origin.x - 32.f * width, origin.y + 32.f * height));
		mCrayonRenderer->drawSegment(cocos2d::Vec2(origin.x - 32.f * width, origin.y + 32.f * height), cocos2d::Vec2(origin.x + 32.f * width, origin.y + 32.f * height));
		mCrayonRenderer->drawSegment(cocos2d::Vec2(origin.x + 32.f * width, origin.y + 32.f * height), cocos2d::Vec2(origin.x + 32.f * width, origin.y - 32.f * height));
		mCrayonRenderer->drawSegment(cocos2d::Vec2(origin.x + 32.f * width, origin.y - 32.f * height), cocos2d::Vec2(origin.x - 32.f * width, origin.y - 32.f * height));
	}

	virtual float getScaleX() const { return mScale.x; }
	virtual void setScaleX(float scale) 
	{ 
		mScale.x = std::abs(scale);
		mNode->setContentSize(cocos2d::Size(64.f * mScale.x, mNode->getContentSize().height));
	}
	virtual float getScaleY() const { return mScale.y; }
	virtual void setScaleY(float scale)
	{
		mScale.y = std::abs(scale);
		mNode->setContentSize(cocos2d::Size(mNode->getContentSize().width, 64.f * mScale.y));
	}

	std::unique_ptr<DynamicCrayonRenderer> mCrayonRenderer;
	cocos2d::Vec2 mScale = cocos2d::Vec2(1.f, 1.f);
};
EditObjectFactory<CrayonBoxEditObject> gTargetPointFactory("CrayonBox", "crayonBox", 5);