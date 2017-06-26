#ifndef __CRAYONPOLYGON_H__
#define __CRAYONPOLYGON_H__

#include "GameObject.h"

class DynamicCrayonRenderer;

class CrayonPolygon : public GameObject
{
public:
	CrayonPolygon(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs);
	virtual ~CrayonPolygon();

	std::unique_ptr<DynamicCrayonRenderer> mCrayonRenderer;
};

#endif