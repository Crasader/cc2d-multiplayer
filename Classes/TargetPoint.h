#ifndef __TARGETPOINT_H__
#define __TARGETPOINT_H__

#include "GameObject.h"

class TargetPoint : public GameObject
{
public:
	TargetPoint(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs);
	virtual ~TargetPoint();
};

#endif