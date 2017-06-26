#ifndef __SPRITEOBJECT_H__
#define __SPRITEOBJECT_H__

#include "GameObject.h"

class SpriteObject : public GameObject
{
public:
	SpriteObject(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs);
	virtual ~SpriteObject();
};

#endif