#ifndef __SCOREBOARD_H__
#define __SCOREBOARD_H__

#include "cocos2d.h"
#include "ui\CocosGUI.h"

class GameSystem;

class Scoreboard
{
public:
	Scoreboard(cocos2d::Node* parent, GameSystem* game);
	~Scoreboard();

	void show(bool show);

	cocos2d::ui::Layout* mLayout;

	GameSystem* mGame;
};

#endif