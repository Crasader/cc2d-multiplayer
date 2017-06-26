#ifndef __MENUBASE_H__
#define __MENUBASE_H__

#include "cocos2d.h"

class GameSystem;

class MenuBase : public cocos2d::Layer
{
public:
	MenuBase();
	virtual ~MenuBase();

	virtual void init(GameSystem* game);

	virtual void setEnableInput(bool enable);

	void addControlButton(cocos2d::MenuItem* menu);

	virtual void onMenuEnter();
	virtual void onMenuBack();
	virtual void onMenuUp();
	virtual void onMenuDown();

	virtual void update(float deltaTime) override;

	virtual void onEnter() override;
	virtual void onExit() override;

	GameSystem* mGame = nullptr;

	bool mInputEnable = false;
	cocos2d::EventListenerKeyboard* mEventListener = nullptr;

	cocos2d::Vector<cocos2d::MenuItem*> mMenus;
	float mUpdateMenuTimer = 0;
};

#endif