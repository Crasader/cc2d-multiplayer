#include "PlayerStart.h"

PlayerStart::PlayerStart(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs)
	: GameObject(game)
{
	mPosition = pos;
}

PlayerStart::~PlayerStart()
{
}

#include "GameObjectFactory.h"
REGISTER_GAMEOBJECT(PlayerStart, GameObject::OT_PlayerStart, "playerStart", false);