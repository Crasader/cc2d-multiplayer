#include "SpriteObject.h"

#include "GameSystem.h"
#include "LevelScene.h"

SpriteObject::SpriteObject(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs)
	: GameObject(game)
{
	std::string fileName = spawnArgs.getString("file", "");
	float scaleX = spawnArgs.getFloat("sx", 1.f);
	float scaleY = spawnArgs.getFloat("sy", 1.f);
	float anchorX = spawnArgs.getFloat("ax", 0.f);
	float anchorY = spawnArgs.getFloat("ay", 0.f);
	int zOrder = spawnArgs.getInt("zOrder", 0);

	auto sprite = cocos2d::Sprite::create(fileName);
	mNode = sprite;
	sprite->setScaleX(scaleX);
	sprite->setScaleY(scaleY);
	sprite->setAnchorPoint(cocos2d::Vec2(anchorX, anchorY));
	sprite->getTexture()->setAliasTexParameters();
	setPosition(pos);
	game->addNode(sprite, zOrder);
}

SpriteObject::~SpriteObject()
{

}

#include "GameObjectFactory.h"
REGISTER_GAMEOBJECT(SpriteObject, GameObject::OT_Sprite, "sprite", false);