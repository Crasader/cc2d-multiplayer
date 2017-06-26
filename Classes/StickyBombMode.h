#ifndef __STICKYBOMBMODE_H__
#define __STICKYBOMBMODE_H__

#include "GameMode.h"

class StickyBomb;
class b2Contact;
struct ContactInfo;

class PlayerStart;

class StickyBombMode : public GameMode
{
public:
	StickyBombMode(GameSystem* game, const std::string& modeName);
	~StickyBombMode();

	virtual void onStart() override;
	virtual void onAllClientsLoadingFinished() override;

	virtual cocos2d::Vec2 getPlayerSpawnSpot(std::uint16_t clientId) override;

	void onBombDestroyed(GameObject* owner);
	void onPlayerContactEnter(GameObject* owner, GameObject* obj, const ContactInfo& contact);

	virtual void onCharacterDestroyed(class Character* character) override;

	virtual void update(float deltaTime) override;

	StickyBomb* mStickyBomb = nullptr;
	float mContactTimer = 0.f;

	float mResetTimer = 0.f;

	std::vector<PlayerStart*> mPlayerStarts;
};

#endif