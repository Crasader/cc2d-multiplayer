#include "CrayonRenderer.h"

CrayonRenderer::CrayonRenderer()
{
	auto s = cocos2d::Director::getInstance()->getWinSize();

	// create a render texture, this is what we are going to draw into
	mRenderTexture = cocos2d::RenderTexture::create(s.width, s.height, cocos2d::Texture2D::PixelFormat::RGBA8888);
	mRenderTexture->retain();
	//mRenderTexture->setPosition(cocos2d::Vec2(s.width / 2, s.height / 2));

	setStyle("pencil_line_black.png");
}

CrayonRenderer::CrayonRenderer(cocos2d::Node* node)
{
	auto s = cocos2d::Director::getInstance()->getWinSize();

	// create a render texture, this is what we are going to draw into
	mRenderTexture = cocos2d::RenderTexture::create(s.width, s.height, cocos2d::Texture2D::PixelFormat::RGBA8888);
	mRenderTexture->retain();
	mRenderTexture->setPosition(cocos2d::Vec2(s.width / 2, s.height / 2));
	node->addChild(mRenderTexture, 3);

	setStyle("pencil_line_black.png");
}

CrayonRenderer::~CrayonRenderer()
{
	mRenderTexture->getParent()->removeChild(mRenderTexture);

	mRenderTexture->release();
	cocos2d::Director::getInstance()->getTextureCache()->removeUnusedTextures();
}

cocos2d::Node* CrayonRenderer::getNode() const
{
	return mRenderTexture;
}

void CrayonRenderer::clear()
{
	mRenderTexture->clear(0, 0, 0, 0);
}

void CrayonRenderer::setStyle(const std::string& name)
{
	if (mSpriteName != name)
	{
		mSpriteName = name;
	}
}

void CrayonRenderer::drawSegment(const cocos2d::Vec2& from, const cocos2d::Vec2& to)
{
	float distance = from.distance(to);
	//float rotation = CC_RADIANS_TO_DEGREES(cocos2d::Vec2::angle(from, to));
	float rotation = -CC_RADIANS_TO_DEGREES((to - from).getAngle());

	if (distance > 1.f)
	{
		cocos2d::RefPtr<cocos2d::Sprite> sprite = cocos2d::Sprite::create(mSpriteName);

		mRenderTexture->begin();
		{
			//cocos2d::Sprite * sprite = cocos2d::Sprite::create(mSpriteName);
			//mBrushs.pushBack(sprite);
			sprite->setAnchorPoint(cocos2d::Vec2(0.f, 0.5f));
			sprite->setPosition(from);
			sprite->setRotation(rotation);
			//sprite->setScaleX(1);
			sprite->getTexture()->setTexParameters({ GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT });
			sprite->setTextureRect(cocos2d::Rect(0, 0, distance, sprite->getTexture()->getContentSize().height));
			sprite->visit();
		}
		mRenderTexture->end();
	}
}