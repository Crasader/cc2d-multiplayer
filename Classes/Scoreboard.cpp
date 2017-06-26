#include "Scoreboard.h"

#include "GameSystem.h"
#include "Lobby.h"
#include "NetworkManager.h"

Scoreboard::Scoreboard(cocos2d::Node* parent, GameSystem* game)
	: mGame(game)
{
	auto winSize = cocos2d::Director::getInstance()->getWinSize();

	mLayout = cocos2d::ui::Layout::create();
	mLayout->setContentSize(cocos2d::Size(700.0f, 500.0f));
	mLayout->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
	mLayout->setLayoutType(cocos2d::ui::LayoutType::VERTICAL);
	mLayout->setBackGroundColorType(cocos2d::ui::Layout::BackGroundColorType::SOLID);
	mLayout->setBackGroundColor(cocos2d::Color3B(128, 128, 128));
	mLayout->setBackGroundColorOpacity(180);

	mLayout->setPosition(cocos2d::Vec2(winSize.width / 2.f, winSize.height / 2.f));
	mLayout->setVisible(false);

	parent->addChild(mLayout, 20);
}

Scoreboard::~Scoreboard()
{
}

void Scoreboard::show(bool show)
{
	if (show)
	{
		mLayout->removeAllChildren();

		cocos2d::ui::LinearLayoutParameter* lp1 = cocos2d::ui::LinearLayoutParameter::create();
		lp1->setGravity(cocos2d::ui::LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);
		lp1->setMargin(cocos2d::ui::Margin(20.0f, 5.0f, 100.0f, 10.0f));

		cocos2d::ui::LinearLayoutParameter* lp2 = cocos2d::ui::LinearLayoutParameter::create();
		lp2->setGravity(cocos2d::ui::LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);
		lp2->setMargin(cocos2d::ui::Margin(100.0f, 5.0f, 100.0f, 10.0f));

		{
			auto horLayout = cocos2d::ui::Layout::create();
			horLayout->setLayoutType(cocos2d::ui::LayoutType::ABSOLUTE);
			horLayout->setContentSize(cocos2d::Size(700.0f, 22.0f));
			mLayout->addChild(horLayout);

			auto nameLabel = cocos2d::ui::Text::create("Name", "fonts/arial.ttf", 20);
			nameLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
			nameLabel->setPosition(cocos2d::Vec2(100, 0));
			horLayout->addChild(nameLabel);

			auto scoreLabel = cocos2d::ui::Text::create("Score", "fonts/arial.ttf", 20);
			scoreLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
			scoreLabel->setPosition(cocos2d::Vec2(300, 0));
			horLayout->addChild(scoreLabel);

			auto latencyLabel = cocos2d::ui::Text::create("Latency", "fonts/arial.ttf", 20);
			latencyLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
			latencyLabel->setPosition(cocos2d::Vec2(500, 0));
			horLayout->addChild(latencyLabel);
		}

		{
			auto horLayout = cocos2d::ui::Layout::create();
			horLayout->setContentSize(cocos2d::Size(700.0f, 22.0f));
			mLayout->addChild(horLayout);
		}

		for (auto &user : mGame->mLobby->mLobbyUsers)
		{
			if (user.isConnected)
			{
				auto horLayout = cocos2d::ui::Layout::create();
				horLayout->setLayoutType(cocos2d::ui::LayoutType::ABSOLUTE);
				horLayout->setContentSize(cocos2d::Size(700.0f, 22.0f));
				mLayout->addChild(horLayout);

				auto nameLabel = cocos2d::ui::Text::create(user.name, "fonts/arial.ttf", 20);
				nameLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
				nameLabel->setPosition(cocos2d::Vec2(100, 0));
				horLayout->addChild(nameLabel);

				std::stringstream ss;
				ss << user.score;
				auto scoreLabel = cocos2d::ui::Text::create(ss.str(), "fonts/arial.ttf", 20);
				scoreLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
				scoreLabel->setPosition(cocos2d::Vec2(300, 0));
				horLayout->addChild(scoreLabel);

				std::stringstream ss2;
				ss2 << user.latency;
				auto latencyLabel = cocos2d::ui::Text::create(ss2.str() , "fonts/arial.ttf", 20);
				latencyLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
				latencyLabel->setPosition(cocos2d::Vec2(500, 0));
				horLayout->addChild(latencyLabel);
			}
		}
	}

	mLayout->setVisible(show);
}