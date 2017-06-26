#ifndef __STARTSERVERMENU_H__
#define __STARTSERVERMENU_H__

#include "MenuBase.h"

#include "ui\CocosGUI.h"

class StartServerMenu : public MenuBase
{
public:
	StartServerMenu();
	~StartServerMenu();

	static cocos2d::Scene* createScene(GameSystem* game);
	bool init();
	void initMenu();

	void onMinus(cocos2d::Ref* sender);
	void onPlus(cocos2d::Ref* sender);

	void onStartServer(cocos2d::Ref* sender);
	void onBack(cocos2d::Ref* sender);

	CREATE_FUNC(StartServerMenu);

private:
	cocos2d::ui::TextField* mServerNameTextField = nullptr;
	cocos2d::ui::Text* mMaxPlayersText = nullptr;
	cocos2d::ui::TextField* mPortTextField = nullptr;
};

#endif