#ifndef __COIN_H__
#define __COIN_H__

#include "GameObject.h"

class Coin : public GameObject
{
public:
	Coin(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs);
	virtual ~Coin();

	virtual void update(float deltaTime) override;
};

#endif