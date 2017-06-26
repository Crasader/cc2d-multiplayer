#ifndef __CRAYONRENDERER_H__
#define __CRAYONRENDERER_H__

#include "cocos2d.h"

class CrayonRenderer
{
public:
	CrayonRenderer();
	CrayonRenderer(cocos2d::Node* node);
	~CrayonRenderer();

	cocos2d::Node* getNode() const;

	void clear();

	void setStyle(const std::string& name);

	void drawSegment(const cocos2d::Vec2& from, const cocos2d::Vec2& to);


	cocos2d::RenderTexture* mRenderTexture;
	std::string mSpriteName;
};

#endif