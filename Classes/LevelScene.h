#ifndef __LEVELSCENE_H__
#define __LEVELSCENE_H__

#include "cocos2d.h"

class GameSystem;
class Input;

class LevelScene : public cocos2d::Layer {
public:
	LevelScene();
	~LevelScene();

	static cocos2d::Scene* createScene();
	virtual bool init();

	void registerInput(Input* input);

	void update(float deltaTime);

	CREATE_FUNC(LevelScene);

	GameSystem* mGame;
	cocos2d::RefPtr<cocos2d::Scene> mScene;

	cocos2d::Layer* mObjectLayer;
};

#endif