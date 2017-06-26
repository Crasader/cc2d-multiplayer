#include "MapSelectorMenu.h"

#include "ui\CocosGUI.h"

#include "json/document.h"
#include "json/filestream.h"

#include "LobbyMenu.h"
#include "GameSystem.h"
#include "NetworkManager.h"

USING_NS_CC;

MapSelectorMenu::MapSelectorMenu()
{

}

MapSelectorMenu::~MapSelectorMenu()
{

}

Scene* MapSelectorMenu::createScene(GameSystem* game, LobbyMenu* lobby)
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = MapSelectorMenu::create();
	layer->mGame = game;
	layer->mOnSelectMap = std::bind(&LobbyMenu::setMapName, lobby, std::placeholders::_1);

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

bool MapSelectorMenu::init()
{
	if (!Layer::init()) {
		return false;
	}

	auto winSize = Director::getInstance()->getWinSize();

	auto backgroundSprite = Sprite::create("Background.png");
	float scalingX = winSize.width / backgroundSprite->getBoundingBox().size.width;
	float scalingY = winSize.height / backgroundSprite->getBoundingBox().size.height;
	backgroundSprite->setScaleX(scalingX);
	backgroundSprite->setScaleY(scalingY);
	backgroundSprite->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
	addChild(backgroundSprite, -1);

	auto textLabel = ui::Text::create("Maps", "fonts/Marker Felt.ttf", 120);
	textLabel->setColor(Color3B::BLACK);
	textLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_TOP);
	textLabel->setPosition(cocos2d::Vec2(winSize.width / 2.f, winSize.height - 10.f));
	addChild(textLabel);

	loadMapNames();

	mMapNameMenus.clear();
	cocos2d::Vector<cocos2d::MenuItem*> items;

	float yOffset = 200;

	for (size_t i = 0; i < mMapNames.size(); ++i)
	{
		auto item = cocos2d::MenuItemFont::create(mMapNames[i], CC_CALLBACK_1(MapSelectorMenu::onSelectMap, this));
		item->setColor(cocos2d::Color3B::BLACK);
		item->setFontSizeObj(40);
		item->setPosition(0, yOffset);

		mMapNameMenus.push_back(item);
		items.pushBack(item);
		
		addControlButton(item);

		yOffset -= 45;
	}

	yOffset -= 20.f;

	auto backButton = cocos2d::MenuItemFont::create("Back", CC_CALLBACK_1(MapSelectorMenu::onBack, this));
	backButton->setColor(cocos2d::Color3B::BLACK);
	backButton->setFontSizeObj(50);
	backButton->setPosition(cocos2d::Vec2(0.f, yOffset));
	addControlButton(backButton);
	items.pushBack(backButton);

	auto menu = cocos2d::Menu::createWithArray(items);
	addChild(menu, 0);

	return true;
}

void MapSelectorMenu::onSelectMap(cocos2d::Ref* sender)
{
	for (size_t i = 0; i < mMapNameMenus.size(); ++i)
	{
		if (mMapNameMenus[i] == sender)
		{
			std::string mapName = mMapNames[i];
			
			if (mOnSelectMap)
				mOnSelectMap(mapName);

			Director::getInstance()->popScene();

			return;
		}
	}
}

void MapSelectorMenu::onBack(cocos2d::Ref* sender)
{
	Director::getInstance()->popScene();
}

void MapSelectorMenu::update(float deltaTime)
{
	MenuBase::update(deltaTime);

	if (mGame && mGame->mNetwork)
	{
		mGame->mNetwork->update(deltaTime);
	}
}

void MapSelectorMenu::loadMapNames()
{
	mMapNames.clear();

	FILE * pFile = fopen("maps.json", "rb");
	rapidjson::FileStream is(pFile);

	rapidjson::Document d;
	d.ParseStream(is);

	rapidjson::Value& maps = d["maps"];
	if (maps.IsArray())
	{
		for (rapidjson::SizeType i = 0; i < maps.Size(); i++)
		{
			rapidjson::Value& mapValue = maps[i];

			std::string mapName = mapValue["name"].GetString();
			mMapNames.push_back(mapName);
		}
	}

	fclose(pFile);
}