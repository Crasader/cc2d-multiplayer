#ifndef __PLAYERSTART_H__
#define __PLAYERSTART_H__

#include "GameObject.h"

class PlayerStart : public GameObject
{
public:
	PlayerStart(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs);
	virtual ~PlayerStart();

	virtual const cocos2d::Vec2 getPosition() const override { return mPosition; }

	cocos2d::Vec2 mPosition;
};

#endif