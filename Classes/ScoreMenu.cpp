#include "ScoreMenu.h"

#include "GameSystem.h"
#include "Lobby.h"
#include "NetworkManager.h"

ScoreMenu::ScoreMenu()
{

}

ScoreMenu::~ScoreMenu()
{

}

cocos2d::Scene* ScoreMenu::createScene(GameSystem* game)
{
	// 'scene' is an autorelease object
	auto scene = cocos2d::Scene::create();

	// 'layer' is an autorelease object
	auto layer = ScoreMenu::create();
	layer->mGame = game;

	layer->showScores();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

bool ScoreMenu::init()
{
	if (!Layer::init())
	{
		return false;
	}

	auto winSize = cocos2d::Director::getInstance()->getWinSize();

	auto backgroundSprite = cocos2d::Sprite::create("Background.png");
	float scalingX = winSize.width / backgroundSprite->getBoundingBox().size.width;
	float scalingY = winSize.height / backgroundSprite->getBoundingBox().size.height;
	backgroundSprite->setScaleX(scalingX);
	backgroundSprite->setScaleY(scalingY);
	backgroundSprite->setAnchorPoint(cocos2d::Vec2::ANCHOR_BOTTOM_LEFT);
	addChild(backgroundSprite, -1);

	auto textLabel = cocos2d::ui::Text::create("Score", "fonts/Marker Felt.ttf", 120);
	textLabel->setColor(cocos2d::Color3B::BLACK);
	textLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_TOP);
	textLabel->setPosition(cocos2d::Vec2(winSize.width / 2.f, winSize.height - 10.f));
	addChild(textLabel);

	scheduleUpdate();

	return true;
}

struct ScoreUser
{
	std::string name;
	std::uint32_t score;
};

struct
{
	bool operator()(ScoreUser& a, ScoreUser& b)
	{
		return a.score > b.score;
	}
} compareScore;

void ScoreMenu::showScores()
{
	auto winSize = cocos2d::Director::getInstance()->getWinSize();

	std::vector<ScoreUser> scoreList;
	for (size_t i = 0; i < mGame->mLobby->mLobbyUsers.size(); ++i)
	{
		if (mGame->mLobby->mLobbyUsers[i].isConnected)
		{
			ScoreUser user;
			user.name = mGame->mLobby->mLobbyUsers[i].name;
			user.score = mGame->mLobby->mLobbyUsers[i].score;
			scoreList.push_back(user);
		}
	}
	std::sort(scoreList.begin(), scoreList.end(), compareScore);

	float offset = 0;

	for (size_t i = 0; i < scoreList.size(); i++)
	{
		std::stringstream ss;
		ss << scoreList[i].score;

		auto playerNameLabel = cocos2d::TextFieldTTF::create();
		playerNameLabel->setColor(cocos2d::Color3B::BLACK);
		playerNameLabel->setString(scoreList[i].name + ": " + ss.str());
		playerNameLabel->setScale(1.5f);
		playerNameLabel->setPosition(cocos2d::Vec2(winSize.width / 2, winSize.height / 2 + offset));
		addChild(playerNameLabel);
		offset -= 20;
	}

	if (mGame->mNetwork->isServer())
	{
		offset -= 40.f;
		auto back = cocos2d::MenuItemFont::create("Back", CC_CALLBACK_1(ScoreMenu::onBack, this));
		back->setColor(cocos2d::Color3B::BLACK);
		back->setFontSizeObj(50);
		back->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
		back->setPosition(cocos2d::Vec2(0, offset));

		auto menu = cocos2d::Menu::create(back, nullptr);
		addChild(menu, 0);
	}
}

void ScoreMenu::onBack(cocos2d::Ref* sender)
{
	if (mGame && mGame->mNetwork->isServer())
	{
		mGame->exitGame();
	}
}

void ScoreMenu::update(float deltaTime)
{
	if (mGame && mGame->mNetwork)
	{
		mGame->mNetwork->update(deltaTime);
	}
}