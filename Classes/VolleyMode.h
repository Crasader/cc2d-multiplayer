#ifndef __VOLLEYMODE_H__
#define __VOLLEYMODE_H__

#include "GameMode.h"

#include <vector>

#include "2d/CCLabel.h"

class GameObject;
struct ContactInfo;

class Ball;

class VolleyMode : public GameMode
{
public:
	struct Team
	{
		std::uint16_t score = 0;
	};

	enum
	{
		TEAM_NONE = -1,
		TEAM_LEFT = 0,
		TEAM_RIGHT = 1
	};

public:
	VolleyMode(GameSystem* game, const std::string& modeName);
	~VolleyMode();

	void restart();

	void resetBall();

	virtual void onStart() override;
	virtual void onAllClientsLoadingFinished() override;

	virtual cocos2d::Vec2 getPlayerSpawnSpot(std::uint16_t clientId) override;

	virtual void onSpawnPlayer(Player* player) override;
	virtual void onCharacterDestroyed(class Character* character) override;

	void onTriggerContactEnter(GameObject* owner, GameObject* obj, const ContactInfo& contact);
	void onPlayerContactEnter(GameObject* owner, GameObject* obj, const ContactInfo& contact);

	void onScore(int team);

	virtual void update(float deltaTime) override;

	virtual bool serialize(NetMessage& msg) override;

	float mResetTimer = 0.f;
	Ball* mResetBall = nullptr;

	std::vector<Team> mTeams;

	int mTeamBallContactCounter = 0;
	int mCurrTeamBallContact = -1;
	int mLastTeamScore = TEAM_NONE;
};

#endif