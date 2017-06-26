#ifndef __SCOREMENU_H__
#define __SCOREMENU_H__

#include "MenuBase.h"

class ScoreMenu : public MenuBase
{
public:
	ScoreMenu();
	virtual ~ScoreMenu();

	static cocos2d::Scene* createScene(GameSystem* game);
	virtual bool init() override;

	void showScores();

	void onBack(cocos2d::Ref* sender);

	virtual void update(float deltaTime) override;

	CREATE_FUNC(ScoreMenu);
};

#endif