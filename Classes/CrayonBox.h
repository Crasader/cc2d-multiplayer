#ifndef __CRAYONBOX_H__
#define __CRAYONBOX_H__

#include "GameObject.h"

class DynamicCrayonRenderer;

class CrayonBox : public GameObject
{
public:
	CrayonBox(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs);
	virtual ~CrayonBox();

	std::unique_ptr<DynamicCrayonRenderer> mCrayonRenderer;
};

#endif