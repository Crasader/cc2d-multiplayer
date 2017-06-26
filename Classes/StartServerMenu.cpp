#include "StartServerMenu.h"

#include "GameSystem.h"
#include "NetworkManager.h"
#include "LobbyMenu.h"

#include <ctype.h>

USING_NS_CC;

StartServerMenu::StartServerMenu()
{

}

StartServerMenu::~StartServerMenu()
{
}

Scene* StartServerMenu::createScene(GameSystem* game)
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = StartServerMenu::create();
	layer->mGame = game;
	layer->initMenu();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

bool StartServerMenu::init()
{
	if (!Layer::init())
	{
		return false;
	}

	return true;
}

void StartServerMenu::initMenu()
{
	auto winSize = Director::getInstance()->getWinSize();

	auto backgroundSprite = Sprite::create("Background.png");
	float scalingX = winSize.width / backgroundSprite->getBoundingBox().size.width;
	float scalingY = winSize.height / backgroundSprite->getBoundingBox().size.height;
	backgroundSprite->setScaleX(scalingX);
	backgroundSprite->setScaleY(scalingY);
	backgroundSprite->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
	addChild(backgroundSprite, -1);

	auto textLabel = ui::Text::create("Start Server", "fonts/Marker Felt.ttf", 120);
	textLabel->setColor(Color3B::BLACK);
	textLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_TOP);
	textLabel->setPosition(cocos2d::Vec2(winSize.width / 2.f, winSize.height - 50.f));
	addChild(textLabel);


	auto serverNameLabel = ui::Text::create("Server name:", "fonts/Marker Felt.ttf", 50);
	serverNameLabel->setColor(Color3B::BLACK);
	serverNameLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_RIGHT);
	serverNameLabel->setPosition(cocos2d::Vec2(winSize.width / 2, winSize.height / 2 + 80));
	addChild(serverNameLabel);

	mServerNameTextField = cocos2d::ui::TextField::create("Server", "fonts/Marker Felt.ttf", 50);
	mServerNameTextField->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
	mServerNameTextField->setTextColor(Color4B::BLACK);
	mServerNameTextField->setTextHorizontalAlignment(TextHAlignment::LEFT);
	mServerNameTextField->setTextVerticalAlignment(TextVAlignment::CENTER);
	mServerNameTextField->setPosition(Vec2(winSize.width / 2.0f + 50.f, winSize.height / 2.0f + 80.f));
	mServerNameTextField->setMaxLengthEnabled(true);
	mServerNameTextField->setMaxLength(64);
	mServerNameTextField->setString(mGame->mNetwork->mServerName);
	addChild(mServerNameTextField);

	auto portLabel = ui::Text::create("Port:", "fonts/Marker Felt.ttf", 50);
	portLabel->setColor(Color3B::BLACK);
	portLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_RIGHT);
	portLabel->setPosition(cocos2d::Vec2(winSize.width / 2, winSize.height / 2 + 20));
	addChild(portLabel);

	std::stringstream ssPort;
	ssPort << NET_DEFAULT_PORT;
	mPortTextField = cocos2d::ui::TextField::create(ssPort.str(), "fonts/Marker Felt.ttf", 50);
	mPortTextField->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
	mPortTextField->setTextColor(Color4B::BLACK);
	mPortTextField->setTextHorizontalAlignment(TextHAlignment::LEFT);
	mPortTextField->setTextVerticalAlignment(TextVAlignment::CENTER);
	mPortTextField->setPosition(Vec2(winSize.width / 2.0f + 50.f, winSize.height / 2.0f + 20.f));
	mPortTextField->setMaxLengthEnabled(true);
	mPortTextField->setMaxLength(6);
	mPortTextField->addEventListener([=](Ref* sender, ui::TextField::EventType type)
	{
		switch (type)
		{
		case ui::TextField::EventType::INSERT_TEXT:
			ui::TextField* textField = static_cast<ui::TextField*>(sender);
			std::string str = textField->getString();

			auto it = str.begin();
			while (it != str.end())
			{
				char c = *it;
				if (!isdigit(c))
				{
					it = str.erase(it);
				}
				else
					++it;
			}

			textField->setString(str);

			break;
		}
	});
	addChild(mPortTextField);

	auto maxPlayersLabel = ui::Text::create("Max Players:", "fonts/Marker Felt.ttf", 50);
	maxPlayersLabel->setColor(Color3B::BLACK);
	maxPlayersLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_RIGHT);
	maxPlayersLabel->setPosition(cocos2d::Vec2(winSize.width / 2, winSize.height / 2 - 40));
	addChild(maxPlayersLabel);


	auto minusButton = cocos2d::MenuItemFont::create("<", CC_CALLBACK_1(StartServerMenu::onMinus, this));
	minusButton->setFontSizeObj(60);
	minusButton->setColor(Color3B::BLACK);
	minusButton->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
	minusButton->setPosition(cocos2d::Vec2(winSize.width / 2.f + 40.f, winSize.height / 2.f - 40.f));
	addControlButton(minusButton);

	std::stringstream ss;
	ss << mGame->mNetwork->mMaxConnections;
	mMaxPlayersText = ui::Text::create(ss.str(), "fonts/Marker Felt.ttf", 50);
	mMaxPlayersText->setColor(Color3B::BLACK);
	mMaxPlayersText->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
	mMaxPlayersText->setPosition(cocos2d::Vec2(winSize.width / 2 + 90, winSize.height / 2 - 40));
	addChild(mMaxPlayersText);

	auto plusButton = cocos2d::MenuItemFont::create(">", CC_CALLBACK_1(StartServerMenu::onPlus, this));
	plusButton->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
	plusButton->setFontSizeObj(60);
	plusButton->setColor(Color3B::BLACK);
	plusButton->setPosition(cocos2d::Vec2(winSize.width / 2.f + 120.f, winSize.height / 2.f - 40.f));
	addControlButton(plusButton);

	auto startServerButton = cocos2d::MenuItemFont::create("Start Server", CC_CALLBACK_1(StartServerMenu::onStartServer, this));
	startServerButton->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	startServerButton->setFontSizeObj(50);
	startServerButton->setColor(Color3B::BLACK);
	startServerButton->setPosition(cocos2d::Vec2(winSize.width / 2.f, winSize.height / 2.f - 240.f));
	startServerButton->selected();
	addControlButton(startServerButton);

	auto backButton = cocos2d::MenuItemFont::create("Back", CC_CALLBACK_1(StartServerMenu::onBack, this));
	backButton->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	backButton->setFontSizeObj(50);
	backButton->setColor(Color3B::BLACK);
	backButton->setPosition(cocos2d::Vec2(winSize.width / 2.f, winSize.height / 2.f - 300.f));
	addControlButton(backButton);

	auto menu = cocos2d::Menu::create(minusButton, plusButton, startServerButton, backButton, nullptr);
	menu->setPosition(cocos2d::Vec2(0, 0));
	addChild(menu);
}

void StartServerMenu::onMinus(cocos2d::Ref* sender)
{
	mGame->mNetwork->mMaxConnections = std::max(--mGame->mNetwork->mMaxConnections, 1U);

	std::stringstream ss;
	ss << mGame->mNetwork->mMaxConnections;
	mMaxPlayersText->setString(ss.str());
}

void StartServerMenu::onPlus(cocos2d::Ref* sender)
{
	mGame->mNetwork->mMaxConnections = std::min(++mGame->mNetwork->mMaxConnections, NET_MAX_CLIENTS);

	std::stringstream ss;
	ss << mGame->mNetwork->mMaxConnections;
	mMaxPlayersText->setString(ss.str());
}

void StartServerMenu::onStartServer(cocos2d::Ref* sender)
{
	std::uint16_t port = NET_DEFAULT_PORT;
	if (mPortTextField)
	{
		std::stringstream ss(mPortTextField->getString());
		ss >> port;
	}

	mGame->mNetwork->mServerName = mServerNameTextField->getString();

	if (mGame->mNetwork->startServer(port))
	{
		Director::getInstance()->pushScene(LobbyMenu::createScene(mGame));
	}
}

void StartServerMenu::onBack(cocos2d::Ref* sender)
{
	Director::getInstance()->popScene();
}