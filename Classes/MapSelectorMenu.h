#ifndef __MAPSELECTORMENU_H__
#define __MAPSELECTORMENU_H__

#include "MenuBase.h"

class LobbyMenu;

class MapSelectorMenu : public MenuBase
{
public:
	MapSelectorMenu();
	~MapSelectorMenu();

	static cocos2d::Scene* createScene(GameSystem* game, LobbyMenu* lobby);

	bool init() override;

	void onSelectMap(cocos2d::Ref* sender);
	void onBack(cocos2d::Ref* sender);

	void update(float deltaTime);

	void loadMapNames();

	CREATE_FUNC(MapSelectorMenu);

	std::vector<std::string> mMapNames;
	std::vector<cocos2d::MenuItem*> mMapNameMenus;

	std::function<void(const std::string&)> mOnSelectMap;
};

#endif