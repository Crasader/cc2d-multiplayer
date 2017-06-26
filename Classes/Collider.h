#ifndef __COLLIDER_H__
#define __COLLIDER_H__

#include "GameObject.h"

class Collider : public GameObject
{
public:
	Collider(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs);
	virtual ~Collider();
};

class ChainCollider : public GameObject
{
public:
	ChainCollider(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs);
	virtual ~ChainCollider();
};

#endif