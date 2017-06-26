#include "DebugDrawer.h"

DebugDrawer::DebugDrawer(cocos2d::Node* node, float ratio)
	: mRatio(ratio)
{
	mDrawNode = cocos2d::DrawNode::create();
	node->addChild(mDrawNode, 10);
}

DebugDrawer::~DebugDrawer()
{

}

void DebugDrawer::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	cocos2d::Vec2* vertices2 = new cocos2d::Vec2[vertexCount];
	for (int i = 0; i<vertexCount; i++)
	{
		vertices2[i].x = vertices[i].x;
		vertices2[i].y = vertices[i].y;

		vertices2[i] *= mRatio;
	}

	mDrawNode->drawPoly(vertices2, vertexCount, false, cocos2d::Color4F(color.r, color.g, color.b, 1.0f));

	delete[] vertices2;
}

void DebugDrawer::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	cocos2d::Vec2* vertices2 = new cocos2d::Vec2[vertexCount];
	for (int i = 0; i<vertexCount; i++)
	{
		vertices2[i].x = vertices[i].x;
		vertices2[i].y = vertices[i].y;

		vertices2[i] *= mRatio;
	}

	mDrawNode->drawPoly(vertices2, vertexCount, true, cocos2d::Color4F(color.r, color.g, color.b, 1.0f));

	delete[] vertices2;
}

void DebugDrawer::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
	mDrawNode->drawCircle(cocos2d::Vec2(center.x, center.y) * mRatio, radius * mRatio, 0.0f, 32, false,
		cocos2d::Color4F(color.r, color.g, color.b, 1.0f));
}

void DebugDrawer::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
	mDrawNode->drawCircle(cocos2d::Vec2(center.x, center.y) * mRatio, radius * mRatio, 0.0f, 32, false,
		cocos2d::Color4F(color.r, color.g, color.b, 1.0f));
}

void DebugDrawer::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	mDrawNode->drawSegment(cocos2d::Vec2(p1.x, p1.y) * mRatio, cocos2d::Vec2(p2.x, p2.y) * mRatio, 0.5f,
		cocos2d::Color4F(color.r, color.g, color.b, 1.0f));
	//mDrawNode->drawLine(cocos2d::Vec2(p1.x, p1.y) * mRatio, cocos2d::Vec2(p2.x, p2.y) * mRatio, cocos2d::Color4F(color.r, color.g, color.b, 1.0f));
}

void DebugDrawer::DrawTransform(const b2Transform& xf)
{
	int a = 0;
}