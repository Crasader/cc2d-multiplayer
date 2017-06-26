#ifndef __STARTMENU_H__
#define __STARTMENU_H__

#include "MenuBase.h"

#include "ui/CocosGUI.h"

class MainMenu : public MenuBase
{
public:
	MainMenu();
	~MainMenu();

	static cocos2d::Scene* createScene(GameSystem* game);
	bool init();
	void initMenu();

	void onCreateServer(cocos2d::Ref* sender);
	void onJoinServer(cocos2d::Ref* sender);
	void onEditor(cocos2d::Ref* sender);

	void onExitGame(cocos2d::Ref* sender);

	void acceptPlayerName();

	void update(float deltaTime);

	virtual void onExit() override;

	CREATE_FUNC(MainMenu);

	cocos2d::ui::TextField* mNameTextField = nullptr;
};

#endif