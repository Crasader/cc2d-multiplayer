#include "VolleyMode.h"

#include "GameSystem.h"
#include "NetworkManager.h"
#include "Physics.h"
#include "Lobby.h"
#include "LevelScene.h"

#include "Player.h"
#include "Ball.h"
#include "TriggerBox.h"
#include "TargetPoint.h"
#include "PlayerStart.h"

#include "Input.h"

VolleyMode::VolleyMode(GameSystem* game, const std::string& modeName)
	: GameMode(game, modeName)
{
	mCurrentState.gameState = GS_WARMUP;

	mTeams.resize(2);

	initScore(2);
}

VolleyMode::~VolleyMode()
{

}

void VolleyMode::restart()
{
	if (!mGame->mNetwork->isServer())
		return;

	checkPlayerRespawn();

	mTeamBallContactCounter = 0;

	mCurrentState.gameState = GS_ACTIVE;
}

void VolleyMode::resetBall()
{
	if (!mGame->mNetwork->isServer())
		return;

	if (mResetBall)
	{
		float middlePosX = (mGame->mLevelWidth / 2.0f) / 32.f;

		cocos2d::Vec2 leftPos;
		cocos2d::Vec2 rightPos;

		std::vector<TargetPoint*> points;
		mGame->findObjects(points);
		for (size_t i = 0; i < points.size(); ++i)
		{
			if (points[i]->getPosition().x < middlePosX)
				leftPos = points[i]->getPosition();
			else
				rightPos = points[i]->getPosition();
		}

		if (mLastTeamScore == TEAM_LEFT)
		{
			mResetBall->setPosition(leftPos);
		}
		else
		{
			mResetBall->setPosition(rightPos);
		}

		mResetBall->mBody->SetLinearVelocity(b2Vec2(0, 0));
		mResetBall->mBody->SetAngularVelocity(0);
		mResetBall->setRotation(0.f);
		mResetBall->mBody->SetAwake(false);

		mResetBall->enableCharacterCollision(true);

		mResetBall = nullptr;

		mLastTeamScore = TEAM_NONE;
	}
}

void VolleyMode::onStart()
{
	mGame->mInput->deactivateAction("rope");

	if (!mGame->mNetwork->isServer())
		return;

	std::vector<TriggerBox*> triggers;
	mGame->findObjects(triggers);
	for (size_t i = 0; i < triggers.size(); ++i)
	{
		triggers[i]->mOnContactEnter.add(this, &VolleyMode::onTriggerContactEnter);
	}
}

void VolleyMode::onAllClientsLoadingFinished()
{
	if (!mGame->mNetwork->isServer())
		return;

	mResetTimer = 5.f;

	restart();
}

cocos2d::Vec2 VolleyMode::getPlayerSpawnSpot(std::uint16_t clientId)
{
	float middlePosX = (mGame->mLevelWidth / 2.0f) / 32.f;

	std::vector<PlayerStart*> teamSpawn[2];

	std::vector<PlayerStart*> points;
	mGame->findObjects(points);
	for (size_t i = 0; i < points.size(); ++i)
	{
		if (points[i]->getPosition().x > middlePosX)
			teamSpawn[TEAM_RIGHT].push_back(points[i]);
		else
			teamSpawn[TEAM_LEFT].push_back(points[i]);
	}

	size_t leftNumPlayers = getNumClientsInTeam(TEAM_LEFT);
	size_t rightNumPlayer = getNumClientsInTeam(TEAM_RIGHT);

	int team = getTeam(clientId);
	if (team == TEAM_NONE)
	{
		team = TEAM_LEFT;
		if (leftNumPlayers == rightNumPlayer)
		{
			team = cocos2d::RandomHelper::random_int(0, 1);
		}
		else if (leftNumPlayers > rightNumPlayer)
		{
			team = TEAM_RIGHT;
		}

		setTeam(clientId, team);
	}

	return teamSpawn[team][cocos2d::RandomHelper::random_int(0, (int)teamSpawn[team].size() - 1)]->getPosition();
}

void VolleyMode::onSpawnPlayer(Player* player)
{
	player->mOnContactEnter.add(this, &VolleyMode::onPlayerContactEnter);
}

void VolleyMode::onCharacterDestroyed(Character* character)
{
	if (dynamic_cast<Player*>(character))
	{
		character->mOnContactEnter.remove(this);
	}
}

void VolleyMode::onTriggerContactEnter(GameObject* owner, GameObject* obj, const ContactInfo& contact)
{
	if (!mGame->mNetwork->isServer() || mResetBall)
		return;

	Ball* ball = dynamic_cast<Ball*>(obj);
	if (ball)
	{
		mResetBall = ball;
		mResetTimer = 3.0f;

		mResetBall->enableCharacterCollision(false);

		float middlePosX = (mGame->mLevelWidth / 2.0f) / 32.f;
		bool isRight = middlePosX < ball->getPosition().x;

		int team = isRight ? TEAM_LEFT : TEAM_RIGHT;
		onScore(team);
	}
}

void VolleyMode::onPlayerContactEnter(GameObject* owner, GameObject* obj, const ContactInfo& contact)
{
	if (!mGame->mNetwork->isServer() || mResetBall)
		return;

	Ball* ball = dynamic_cast<Ball*>(obj);
	if (ball)
	{
		int team = getTeam(dynamic_cast<Player*>(owner));
		if (team != TEAM_NONE)
		{
			if (mCurrTeamBallContact != team)
				mTeamBallContactCounter = 0;
			mCurrTeamBallContact = team;
			mTeamBallContactCounter++;

			if (mTeamBallContactCounter >= 4)
			{
				if (!mResetBall)
				{
					mResetBall = ball;
					mResetTimer = 3.0f;

					mResetBall->enableCharacterCollision(false);

					int otherTeam = team == TEAM_LEFT ? TEAM_RIGHT : TEAM_LEFT;
					onScore(otherTeam);
				}
			}
		}
	}
}

void VolleyMode::onScore(int team)
{
	mTeams[team].score++;
	setScore(team, mTeams[team].score);

	mLastTeamScore = team;

	if (mTeams[team].score >= 5)
	{
		mGame->scoreTeam(team);

		mGame->setGameOver();
	}
}

void VolleyMode::update(float deltaTime)
{
	GameMode::update(deltaTime);

	if (!mGame->mNetwork->isServer() || mGame->isGameOver())
		return;

	if (mResetTimer > 0)
	{
		mResetTimer -= deltaTime;

		setCountdown(mResetTimer + 1);
		showCountdown(true);

		if (mResetTimer <= 0.f)
		{
			showCountdown(false);
			mResetTimer = 0.0f;

			if (!mResetBall)
			{
				cocos2d::Vec2 spawnPos(4, 9);
				std::vector<TargetPoint*> points;
				mGame->findObjects(points);
				for (size_t i = 0; i < points.size(); ++i)
				{
					spawnPos = points[i]->getPosition();
					break;
				}

				Parameters spawnPara;
				spawnPara.setString("sprite", "Ball.png");
				spawnPara.setFloat("scaleX", 1.5f);
				spawnPara.setFloat("scaleY", 1.5f);
				spawnPara.setFloat("restitution", 0.98f);
				spawnPara.setFloat("density", 0.2f);
				spawnPara.setFloat("linearDamping", 0.1f);
				spawnPara.setFloat("radius", 1.5f);
				spawnPara.setFloat("offsetX", 0.f);
				spawnPara.setFloat("offsetY", 0.0f);
				spawnPara.setFloat("maxVelocity", 20.f);
				spawnPara.setFloat("impulseX", 30.f);
				spawnPara.setFloat("impulseY", 40.f);
				Ball* ball = static_cast<Ball*>(mGame->spawnGameObject("ball", spawnPos, spawnPara));
				ball->mBody->SetAwake(false);
			}
			else
			{
				resetBall();
			}

			restart();
		}
	}

	checkPlayerRespawn(5000);
}

bool VolleyMode::serialize(NetMessage& msg)
{
	if (!GameMode::serialize(msg))
		return false;

	return true;
}

#include "GameModeFactory.h"
REGISTER_GAMEMODE(VolleyMode, "volley")