#ifndef __DYNAMICCRAYONRENDERER_H__
#define __DYNAMICCRAYONRENDERER_H__

#include "cocos2d.h"

class DynamicCrayonRenderer
{
public:
	DynamicCrayonRenderer();
	~DynamicCrayonRenderer();

	cocos2d::Node* getNode() const;

	void clear();

	void setStyle(const std::string& name, float scale = 0.75f);

	void drawSegment(const cocos2d::Vec2& from, const cocos2d::Vec2& to);
	void updateSegment(size_t index, const cocos2d::Vec2& from, const cocos2d::Vec2& to);

	cocos2d::RefPtr<cocos2d::Node> mNode;

	std::string mSpriteName;
	float mScale = 1.f;
	std::vector<cocos2d::RefPtr<cocos2d::Sprite>> mSprites;
};

#endif