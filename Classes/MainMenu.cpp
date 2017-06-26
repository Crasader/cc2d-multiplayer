#include "MainMenu.h"

#include "GameSystem.h"
#include "NetworkManager.h"
#include "StartServerMenu.h"
#include "ServerBrowser.h"
#include "Editor.h"

USING_NS_CC;

MainMenu::MainMenu()
{

}

MainMenu::~MainMenu()
{
}

Scene* MainMenu::createScene(GameSystem* game)
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = MainMenu::create();
	layer->mGame = game;
	layer->initMenu();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

bool MainMenu::init()
{
	if (!Layer::init())
	{
		return false;
	}

	auto winSize = cocos2d::Director::getInstance()->getWinSize();

	auto backgroundSprite = Sprite::create("Background.png");
	float scalingX = winSize.width / backgroundSprite->getBoundingBox().size.width;
	float scalingY = winSize.height / backgroundSprite->getBoundingBox().size.height;
	backgroundSprite->setScaleX(scalingX);
	backgroundSprite->setScaleY(scalingY);
	backgroundSprite->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
	addChild(backgroundSprite, -1);

	cocos2d::MenuItemFont::setFontName("Marker Felt.ttf");

	auto textLabel = ui::Text::create("Multiplayer", "fonts/Marker Felt.ttf", 120);
	textLabel->setColor(Color3B::BLACK);
	textLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_TOP);
	textLabel->setPosition(cocos2d::Vec2(winSize.width / 2.f, winSize.height - 100.f));
	addChild(textLabel);

	auto createServer = cocos2d::MenuItemFont::create("Create Server", CC_CALLBACK_1(MainMenu::onCreateServer, this));
	createServer->setFontSizeObj(50);
	createServer->setColor(Color3B::BLACK);
	createServer->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
	createServer->setPosition(0, 60);
	createServer->selected();
	addControlButton(createServer);

	auto joinServer = cocos2d::MenuItemFont::create("Join Server", CC_CALLBACK_1(MainMenu::onJoinServer, this));
	joinServer->setFontSizeObj(50);
	joinServer->setColor(Color3B::BLACK);
	joinServer->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
	joinServer->setPosition(0, 0);
	addControlButton(joinServer);

	auto editor = cocos2d::MenuItemFont::create("Editor", CC_CALLBACK_1(MainMenu::onEditor, this));
	editor->setFontSizeObj(50);
	editor->setColor(Color3B::BLACK);
	editor->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
	editor->setPosition(0, -60);
	addControlButton(editor);

	auto exit = cocos2d::MenuItemFont::create("Exit", CC_CALLBACK_1(MainMenu::onExitGame, this));
	exit->setFontSizeObj(50);
	exit->setColor(Color3B::BLACK);
	exit->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
	exit->setPosition(0, -200);
	addControlButton(exit);

	auto menu = cocos2d::Menu::create(createServer, joinServer, editor, exit, nullptr);
	addChild(menu, 0);

	mNameTextField = cocos2d::ui::TextField::create("Name", "fonts/Marker Felt.ttf", 40);
	mNameTextField->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	mNameTextField->setColor(Color3B::BLACK);
	//mNameTextField->setTextAreaSize(Size(240, 20));
	mNameTextField->setTextHorizontalAlignment(TextHAlignment::CENTER);
	mNameTextField->setTextVerticalAlignment(TextVAlignment::CENTER);
	mNameTextField->setPosition(Vec2(winSize.width / 2.f, winSize.height / 2.f - 320.f));
	addChild(mNameTextField);

	scheduleUpdate();

	return true;
}

void MainMenu::initMenu()
{
	std::string playerName = mGame->mConfig.getString("playerName");
	if (playerName.empty())
	{
		mNameTextField->setString("Player");
	}
	else
	{
		mNameTextField->setString(playerName);
	}
}

void MainMenu::onCreateServer(cocos2d::Ref* sender)
{
	Director::getInstance()->pushScene(StartServerMenu::createScene(mGame));
}

void MainMenu::onJoinServer(cocos2d::Ref* sender)
{
	Director::getInstance()->pushScene(ServerBrowser::createScene(mGame));
}

void MainMenu::onEditor(cocos2d::Ref* sender)
{
	Director::getInstance()->pushScene(Editor::createScene());
}

void MainMenu::onExitGame(cocos2d::Ref* sender)
{
	cocos2d::Director::getInstance()->end();
}

void MainMenu::acceptPlayerName()
{
	mGame->mNetwork->mPlayerName = mNameTextField->getString();
	if (mGame->mNetwork->mPlayerName.empty())
		mGame->mNetwork->mPlayerName = "Player";

	bool changed = (mGame->mConfig.getString("playerName") != mGame->mNetwork->mPlayerName);
	{
		mGame->mConfig.setString("playerName", mGame->mNetwork->mPlayerName);
		mGame->writeConfig();
	}
}

void MainMenu::update(float deltaTime)
{
	MenuBase::update(deltaTime);

	if (mGame->mNetwork.get())
	{
		mGame->mNetwork->update(deltaTime);
	}
}

void MainMenu::onExit()
{
	acceptPlayerName();

	MenuBase::onExit();
}