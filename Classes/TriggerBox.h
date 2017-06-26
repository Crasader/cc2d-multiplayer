#ifndef __TRIGGERBOX_H__
#define __TRIGGERBOX_H__

#include "GameObject.h"

#include "MulticastDelegate.h"

class TriggerBox : public GameObject
{
public:
	TriggerBox(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs);
	virtual ~TriggerBox();
};

#endif