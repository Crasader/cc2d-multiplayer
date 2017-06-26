#include "EditorMenus.h"

#include "Editor.h"
#include "EditorObjects.h"

#include "GameModeFactory.h"

EditorCreateMenu::EditorCreateMenu()
{
}

EditorCreateMenu::~EditorCreateMenu()
{
}

cocos2d::Scene* EditorCreateMenu::createScene(Editor* editor)
{
	auto scene = cocos2d::Scene::create();

	auto layer = EditorCreateMenu::create();
	layer->mScene = scene;
	layer->mEditor = editor;

	scene->addChild(layer);

	return scene;
}

bool EditorCreateMenu::init()
{
	if (!Layer::init()) {
		return false;
	}

	auto origin = cocos2d::Director::getInstance()->getVisibleOrigin();
	auto winSize = cocos2d::Director::getInstance()->getWinSize();

	cocos2d::Vector<cocos2d::MenuItem*> items;
	int yOffset = (winSize.height / 2.0f) - 20.0f;

	mSpriteItem = cocos2d::MenuItemFont::create("Sprite", CC_CALLBACK_1(EditorCreateMenu::onCreateObject, this));
	mSpriteItem->setFontSizeObj(20);
	mSpriteItem->setPosition(0, yOffset);
	items.pushBack(mSpriteItem);
	yOffset -= mSpriteItem->getContentSize().height + 5.0f;

	auto it = EditObjectDatabase::getInstance()->mFactorys.begin();
	auto end = EditObjectDatabase::getInstance()->mFactorys.end();
	while (it != end)
	{
		registerObject(it->second->mName, it->second->mType, it->second->mZOrder);
		++it;
	}
	for (size_t i = 0; i < mObjectCreaters.size(); ++i)
	{
		mObjectCreaters[i].item->setFontSizeObj(20);
		mObjectCreaters[i].item->setPosition(0, yOffset);
		items.pushBack(mObjectCreaters[i].item);
		yOffset -= mObjectCreaters[i].item->getContentSize().height + 5.0f;
	}

	yOffset -= 30.0f;
	auto backItem = cocos2d::MenuItemFont::create("Back", CC_CALLBACK_1(EditorCreateMenu::onBack, this));
	backItem->setFontSizeObj(40);
	backItem->setPosition(0, yOffset);
	items.pushBack(backItem);
	yOffset -= backItem->getContentSize().height + 5.0f;

	auto menu = cocos2d::Menu::createWithArray(items);
	menu->setPosition(winSize.width / 2.0f, winSize.height / 2.0f);
	addChild(menu);

	return true;
}

void EditorCreateMenu::onEnter()
{
	Layer::onEnter();
}

void EditorCreateMenu::onExit()
{
	Layer::onExit();
}

void EditorCreateMenu::registerObject(const std::string& name, const std::string& type, int zOrder)
{
	cocos2d::MenuItemFont* item = cocos2d::MenuItemFont::create(name, CC_CALLBACK_1(EditorCreateMenu::onCreateObject, this));
	mObjectCreaters.emplace_back(item, name, type, zOrder);
}

void EditorCreateMenu::onCreateObject(cocos2d::Ref* sender)
{
	if (mSpriteItem == sender)
	{
		cocos2d::Director::getInstance()->pushScene(EditorSpriteCreateMenu::createScene(mEditor));
		return;
	}
	else
	{
		for (size_t i = 0; i < mObjectCreaters.size(); ++i)
		{
			if (mObjectCreaters[i].item == sender)
			{
				mEditor->mIsCreatingObject = true;
				mEditor->mCreateObjectType = mObjectCreaters[i].type;
				mEditor->mCreateObjectZOrder = mObjectCreaters[i].zOrder;
				mEditor->mCreateObjectFileName = "";
				mEditor->mDuplicateObject.enable = false;

				cocos2d::Director::getInstance()->popScene();
				return;
			}
		}
	}
}

void EditorCreateMenu::onBack(cocos2d::Ref* sender)
{
	cocos2d::Director::getInstance()->popScene();
}

cocos2d::Scene* EditorSpriteCreateMenu::createScene(Editor* editor)
{
	auto scene = cocos2d::Scene::create();

	auto layer = EditorSpriteCreateMenu::create();
	layer->mScene = scene;
	layer->mEditor = editor;

	scene->addChild(layer);

	return scene;
}

bool EditorSpriteCreateMenu::init()
{
	if (!Layer::init()) {
		return false;
	}

	auto origin = cocos2d::Director::getInstance()->getVisibleOrigin();
	auto winSize = cocos2d::Director::getInstance()->getWinSize();

	mTextField = cocos2d::ui::TextField::create("Sprite file", "fonts/Marker Felt.ttf", 50);
	mTextField->setTextHorizontalAlignment(cocos2d::TextHAlignment::CENTER);
	mTextField->setTextVerticalAlignment(cocos2d::TextVAlignment::CENTER);
	mTextField->setPosition(cocos2d::Vec2(winSize.width / 2.0f, winSize.height / 2.0f + 20.f));
	addChild(mTextField);

	float yOffset = -20.f;
	mCurrZOrder = 0;
	{
		auto item = cocos2d::MenuItemFont::create("zOrder 0", CC_CALLBACK_1(EditorSpriteCreateMenu::onSetZOrder, this));
		item->setFontSizeObj(20);
		item->setPosition(0, yOffset);
		mZOrderMenus.pushBack(item);
		yOffset -= 20.0f;
	}
	{
		auto item = cocos2d::MenuItemFont::create("zOrder 1", CC_CALLBACK_1(EditorSpriteCreateMenu::onSetZOrder, this));
		item->setFontSizeObj(20);
		item->setPosition(0, yOffset);
		mZOrderMenus.pushBack(item);
		yOffset -= 20.0f;
	}
	{
		auto item = cocos2d::MenuItemFont::create("zOrder 2", CC_CALLBACK_1(EditorSpriteCreateMenu::onSetZOrder, this));
		item->setFontSizeObj(20);
		item->setPosition(0, yOffset);
		mZOrderMenus.pushBack(item);
		yOffset -= 20.0f;
	}
	{
		auto item = cocos2d::MenuItemFont::create("zOrder 3", CC_CALLBACK_1(EditorSpriteCreateMenu::onSetZOrder, this));
		item->setFontSizeObj(20);
		item->setPosition(0, yOffset);
		mZOrderMenus.pushBack(item);
		yOffset -= 20.0f;
	}
	{
		auto item = cocos2d::MenuItemFont::create("zOrder 4", CC_CALLBACK_1(EditorSpriteCreateMenu::onSetZOrder, this));
		item->setFontSizeObj(20);
		item->setPosition(0, yOffset);
		mZOrderMenus.pushBack(item);
		yOffset -= 20.0f;
	}
	{
		auto item = cocos2d::MenuItemFont::create("zOrder 5", CC_CALLBACK_1(EditorSpriteCreateMenu::onSetZOrder, this));
		item->setFontSizeObj(20);
		item->setPosition(0, yOffset);
		mZOrderMenus.pushBack(item);
		yOffset -= 20.0f;
	}

	yOffset -= 100.0f;

	auto okItem = cocos2d::MenuItemFont::create("Ok", CC_CALLBACK_1(EditorSpriteCreateMenu::onCreateSprite, this));
	okItem->setFontSizeObj(40);
	okItem->setPosition(0, yOffset);
	mZOrderMenus.pushBack(okItem);
	yOffset -= 40.0f;

	auto backItem = cocos2d::MenuItemFont::create("Back", CC_CALLBACK_1(EditorSpriteCreateMenu::onBack, this));
	backItem->setFontSizeObj(40);
	backItem->setPosition(0, yOffset);
	mZOrderMenus.pushBack(backItem);
	yOffset -= 40.0f;

	auto menu = cocos2d::Menu::createWithArray(mZOrderMenus);
	menu->setPosition(winSize.width / 2.0f, winSize.height / 2.0f);
	addChild(menu);

	return true;
}

void EditorSpriteCreateMenu::onEnter()
{
	Layer::onEnter();
}

void EditorSpriteCreateMenu::onExit()
{
	Layer::onExit();
}

void EditorSpriteCreateMenu::onSetZOrder(cocos2d::Ref* sender)
{
	for (ssize_t i = 0; i < mZOrderMenus.size(); ++i)
	{
		if (mZOrderMenus.at(i) == sender)
		{
			mCurrZOrder = i;
			return;
		}
	}
}

void EditorSpriteCreateMenu::onCreateSprite(cocos2d::Ref* sender)
{
	std::string filename = mTextField->getString();
	if (!cocos2d::FileUtils::getInstance()->isFileExist(filename))
		return;

	mEditor->mIsCreatingObject = true;
	mEditor->mCreateObjectType = "sprite";
	mEditor->mCreateObjectZOrder = mCurrZOrder;
	mEditor->mCreateObjectFileName = filename;
	mEditor->mDuplicateObject.enable = false;

	cocos2d::Director::getInstance()->popScene();
	cocos2d::Director::getInstance()->popScene();
}

void EditorSpriteCreateMenu::onBack(cocos2d::Ref* sender)
{
	cocos2d::Director::getInstance()->popScene();
}


cocos2d::Scene* EditorLoadMapMenu::createScene(Editor* editor)
{
	auto scene = cocos2d::Scene::create();

	auto layer = EditorLoadMapMenu::create();
	layer->mScene = scene;
	layer->mEditor = editor;
	layer->initMenus();

	scene->addChild(layer);

	return scene;
}

bool EditorLoadMapMenu::init()
{
	if (!Layer::init()) {
		return false;
	}

	return true;
}

void EditorLoadMapMenu::initMenus()
{
	auto origin = cocos2d::Director::getInstance()->getVisibleOrigin();
	auto winSize = cocos2d::Director::getInstance()->getWinSize();

	cocos2d::Vector<cocos2d::MenuItem*> items;
	mMapNamesMenus.clear();

	float yOffset = (winSize.height / 2.0f) - 20.0f;

	auto it = mEditor->mMapNames.begin();
	auto end = mEditor->mMapNames.end();
	while (it != end)
	{
		cocos2d::MenuItemFont* item = cocos2d::MenuItemFont::create((*it), CC_CALLBACK_1(EditorLoadMapMenu::onLoad, this));
		item->setFontSizeObj(20);
		item->setPosition(0, yOffset);
		yOffset -= item->getContentSize().height + 5.0f;

		mMapNamesMenus.push_back(item);
		items.pushBack(item);
		++it;
	}

	yOffset -= 20.f;
	auto backItem = cocos2d::MenuItemFont::create("Back", CC_CALLBACK_1(EditorLoadMapMenu::onBack, this));
	backItem->setFontSizeObj(40);
	backItem->setPosition(0, yOffset);
	items.pushBack(backItem);

	auto menu = cocos2d::Menu::createWithArray(items);
	menu->setPosition(winSize.width / 2.0f, winSize.height / 2.0f);
	addChild(menu);
}

void EditorLoadMapMenu::onEnter()
{
	Layer::onEnter();
}

void EditorLoadMapMenu::onExit()
{
	Layer::onExit();
}

void EditorLoadMapMenu::onLoad(cocos2d::Ref* sender)
{
	for (size_t i = 0; i < mMapNamesMenus.size(); ++i)
	{
		if (mMapNamesMenus[i] == sender)
		{
			std::string mapName = mEditor->mMapNames[i];

			cocos2d::Director::getInstance()->popScene();

			mEditor->loadMap(mapName);

			return;
		}
	}
}

void EditorLoadMapMenu::onBack(cocos2d::Ref* sender)
{
	cocos2d::Director::getInstance()->popScene();
}


cocos2d::Scene* EditorSaveMapMenu::createScene(Editor* editor)
{
	auto scene = cocos2d::Scene::create();

	auto layer = EditorSaveMapMenu::create();
	layer->mScene = scene;
	layer->mEditor = editor;

	scene->addChild(layer);

	return scene;
}

bool EditorSaveMapMenu::init()
{
	if (!Layer::init()) {
		return false;
	}

	auto origin = cocos2d::Director::getInstance()->getVisibleOrigin();
	auto winSize = cocos2d::Director::getInstance()->getWinSize();

	mTextField = cocos2d::ui::TextField::create("Map name", "fonts/Marker Felt.ttf", 30);
	mTextField->setTextHorizontalAlignment(cocos2d::TextHAlignment::CENTER);
	mTextField->setTextVerticalAlignment(cocos2d::TextVAlignment::CENTER);
	mTextField->setPosition(cocos2d::Vec2(winSize.width / 2.0f, winSize.height / 2.0f));
	mTextField->setMaxLengthEnabled(true);
	mTextField->setMaxLength(120);
	addChild(mTextField);

	auto saveItem = cocos2d::MenuItemFont::create("Save", CC_CALLBACK_1(EditorSaveMapMenu::onSave, this));
	saveItem->setFontSizeObj(40);
	saveItem->setPosition(0, -200);

	auto backItem = cocos2d::MenuItemFont::create("Back", CC_CALLBACK_1(EditorSaveMapMenu::onBack, this));
	backItem->setFontSizeObj(40);
	backItem->setPosition(0, -250);

	auto menu = cocos2d::Menu::create(saveItem, backItem, nullptr);
	menu->setPosition(winSize.width / 2.0f, winSize.height / 2.0f);
	addChild(menu);

	return true;
}

void EditorSaveMapMenu::onEnter()
{
	Layer::onEnter();
}

void EditorSaveMapMenu::onExit()
{
	Layer::onExit();
}

void EditorSaveMapMenu::onSave(cocos2d::Ref* sender)
{
	std::string mapName = mTextField->getString();
	if (mapName.empty())
		return;

	mEditor->mMapName = mapName;
	mEditor->mFirstSave = false;

	if (mEditor->mMapNames.empty())
	{
		mEditor->loadMapNames();
	}
	mEditor->mMapNames.push_back(mapName);
	mEditor->saveMapNames();

	mEditor->saveMap(mapName);

	cocos2d::Director::getInstance()->popScene();
}

void EditorSaveMapMenu::onBack(cocos2d::Ref* sender)
{
	cocos2d::Director::getInstance()->popScene();
}


EditorModeMenu::EditorModeMenu()
{
}

EditorModeMenu::~EditorModeMenu()
{
}

cocos2d::Scene* EditorModeMenu::createScene(Editor* editor)
{
	auto scene = cocos2d::Scene::create();

	auto layer = EditorModeMenu::create();
	layer->mScene = scene;
	layer->mEditor = editor;

	scene->addChild(layer);

	return scene;
}

bool EditorModeMenu::init()
{
	if (!Layer::init()) {
		return false;
	}

	auto it = GameModeDatabase::getInstance()->mNameFactorys.begin();
	auto end = GameModeDatabase::getInstance()->mNameFactorys.end();
	while (it != end)
	{
		registerMode(it->second->mClassName);
		++it;
	}

	auto origin = cocos2d::Director::getInstance()->getVisibleOrigin();
	auto winSize = cocos2d::Director::getInstance()->getWinSize();

	cocos2d::Vector<cocos2d::MenuItem*> items;
	int yOffset = (winSize.height / 2.0f) - 20.0f;

	for (size_t i = 0; i < mModes.size(); ++i)
	{
		mModes[i].item->setFontSizeObj(20);
		mModes[i].item->setPosition(0, yOffset);
		items.pushBack(mModes[i].item);
		yOffset -= 20.0f;
	}

	yOffset -= 20.0f;
	auto backItem = cocos2d::MenuItemFont::create("Back", CC_CALLBACK_1(EditorModeMenu::onBack, this));
	backItem->setFontSizeObj(40);
	backItem->setPosition(0, yOffset);
	items.pushBack(backItem);
	yOffset -= 40.0f;

	auto menu = cocos2d::Menu::createWithArray(items);
	menu->setPosition(winSize.width / 2.0f, winSize.height / 2.0f);
	addChild(menu);

	return true;
}

void EditorModeMenu::onEnter()
{
	Layer::onEnter();
}

void EditorModeMenu::onExit()
{
	Layer::onExit();
}

void EditorModeMenu::registerMode(const std::string& name)
{
	std::string newName = name;
	if (newName.empty())
		newName = "None";
	cocos2d::MenuItemFont* item = cocos2d::MenuItemFont::create(newName, CC_CALLBACK_1(EditorModeMenu::onSetMode, this));
	item->setFontSizeObj(20);
	mModes.emplace_back(item, name);
}

void EditorModeMenu::onSetMode(cocos2d::Ref* sender)
{
	for (size_t i = 0; i < mModes.size(); ++i)
	{
		if (mModes[i].item == sender)
		{
			mEditor->mModeName = mModes[i].name;

			cocos2d::Director::getInstance()->popScene();
			return;
		}
	}
}

void EditorModeMenu::onBack(cocos2d::Ref* sender)
{
	cocos2d::Director::getInstance()->popScene();
}