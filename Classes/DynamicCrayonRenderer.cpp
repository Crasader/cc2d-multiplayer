#include "DynamicCrayonRenderer.h"

DynamicCrayonRenderer::DynamicCrayonRenderer()
{
	mNode = cocos2d::Node::create();
	mNode->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);

	setStyle("pencil_line_black.png");
}

DynamicCrayonRenderer::~DynamicCrayonRenderer()
{
	clear();
}

cocos2d::Node* DynamicCrayonRenderer::getNode() const
{
	return mNode;
}

void DynamicCrayonRenderer::clear()
{
	mSprites.clear();
	mNode->removeAllChildrenWithCleanup(true);
}

void DynamicCrayonRenderer::setStyle(const std::string& name, float scale)
{
	if (mSpriteName != name)
	{
		mSpriteName = name;
	}

	mScale = scale;
}

void DynamicCrayonRenderer::drawSegment(const cocos2d::Vec2& from, const cocos2d::Vec2& to)
{
	float distance = from.distance(to);
	float rotation = -CC_RADIANS_TO_DEGREES((to - from).getAngle());

	if (distance > 1.f)
	{
		cocos2d::RefPtr<cocos2d::Sprite> sprite = cocos2d::Sprite::create(mSpriteName);
		sprite->getTexture()->setTexParameters({ GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT });
		sprite->setAnchorPoint(cocos2d::Vec2(0.f, 0.5f));
		sprite->setPosition(from);

		auto rect = sprite->getTextureRect();
		rect.size.width = distance;
		sprite->setTextureRect(rect);
		sprite->setScaleY(mScale);
		sprite->setRotation(rotation);

		mNode->addChild(sprite);

		mSprites.push_back(sprite);
	}
}

void DynamicCrayonRenderer::updateSegment(size_t index, const cocos2d::Vec2& from, const cocos2d::Vec2& to)
{
	if (index >= mSprites.size())
		return;

	float distance = from.distance(to);
	float rotation = -CC_RADIANS_TO_DEGREES((to - from).getAngle());

	auto sprite = mSprites[index];
	sprite->setPosition(from);
	auto rect = sprite->getTextureRect();
	rect.size.width = distance;
	sprite->setTextureRect(rect);
	sprite->setRotation(rotation);
}