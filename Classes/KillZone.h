#ifndef __KILLZONE_H__
#define __KILLZONE_H__

#include "GameObject.h"

class KillZone : public GameObject
{
public:
	KillZone(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs);
	virtual ~KillZone();

	virtual void onContactEnter(GameObject* obj, const ContactInfo& contact) override;
};

#endif