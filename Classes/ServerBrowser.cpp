#include "ServerBrowser.h"

#include "ui\CocosGUI.h"

#include <vector>

#include <enet/enet.h>
#include <enet/time.h>

#include "GameSystem.h"
#include "NetworkManager.h"

#include "LobbyMenu.h"

#include "MasterClient.h"
#include "SearchLanHost.h"

USING_NS_CC;

Scene* ServerBrowser::createScene(GameSystem* game)
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = ServerBrowser::create();
	layer->init(game);

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

bool ServerBrowser::init()
{
	if (!Layer::init())
	{
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



	mTabControl = cocos2d::ui::TabControl::create();
	mTabControl->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	mTabControl->setPosition(Vec2(winSize.width / 2.f, winSize.height / 2.f + 100.f));
	mTabControl->setContentSize(Size(600.f, 500.f));
	mTabControl->setHeaderWidth(100.f);
	addChild(mTabControl);

	{
		cocos2d::ui::Layout* layout = cocos2d::ui::Layout::create();
		layout->setLayoutType(cocos2d::ui::LayoutType::ABSOLUTE);
		layout->setContentSize(Size(mTabControl->getContentSize().width, mTabControl->getContentSize().height));

		{
			{
				auto text = cocos2d::ui::Text::create("Name", "fonts / Marker Felt.ttf", 10);
				text->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
				text->setColor(Color3B::BLACK);
				text->setPosition(Vec2(0, 460));
				text->setContentSize(Size(260, 20));
				layout->addChild(text);
			}

			{
				auto text = cocos2d::ui::Text::create("Latency", "fonts / Marker Felt.ttf", 10);
				text->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
				text->setColor(Color3B::BLACK);
				text->setPosition(Vec2(260, 460));
				text->setContentSize(Size(50, 20));
				layout->addChild(text);
			}

			{
				auto text = cocos2d::ui::Text::create("Map", "fonts / Marker Felt.ttf", 10);
				text->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
				text->setColor(Color3B::BLACK);
				text->setPosition(Vec2(310, 460));
				text->setContentSize(Size(100, 20));
				layout->addChild(text);
			}

			{
				auto text = cocos2d::ui::Text::create("Modus", "fonts / Marker Felt.ttf", 10);
				text->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
				text->setColor(Color3B::BLACK);
				text->setPosition(Vec2(410, 460));
				text->setContentSize(Size(100, 20));
				layout->addChild(text);
			}

			{
				auto text = cocos2d::ui::Text::create("Num", "fonts / Marker Felt.ttf", 10);
				text->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
				text->setColor(Color3B::BLACK);
				text->setPosition(Vec2(510, 460));
				text->setContentSize(Size(20, 20));
				layout->addChild(text);
			}

			{
				auto text = cocos2d::ui::Text::create("Max", "fonts / Marker Felt.ttf", 10);
				text->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
				text->setColor(Color3B::BLACK);
				text->setPosition(Vec2(530, 460));
				text->setContentSize(Size(20, 20));
				layout->addChild(text);
			}
		}

		mInternetListView = cocos2d::ui::ListView::create();
		mInternetListView->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
		mInternetListView->setPosition(Point(0, 0));
		mInternetListView->setContentSize(Size(600.f, 450.f));
		mInternetListView->setTouchEnabled(false);
		//mInternetListView->setClippingEnabled(true);
		mInternetListView->setGravity(cocos2d::ui::ListView::Gravity::CENTER_HORIZONTAL);
		mInternetListView->setBackGroundColorType(cocos2d::ui::Layout::BackGroundColorType::SOLID);
		mInternetListView->setBackGroundColor(cocos2d::Color3B(128, 128, 128));
		mInternetListView->setBackGroundColorOpacity(140);
		mInternetListView->addEventListener((ui::ListView::ccListViewCallback)CC_CALLBACK_2(ServerBrowser::onSelectedItemEvent, this));
		layout->addChild(mInternetListView);

		auto tabInternetHeader = cocos2d::ui::TabHeader::create();
		tabInternetHeader->setTitleText("Internet");
		tabInternetHeader->setTitleFontSize(30.f);
		tabInternetHeader->setColor(Color3B::BLACK);
		tabInternetHeader->setContentSize(Size(400.f, 32.f));
		mTabControl->insertTab(0, tabInternetHeader, layout);
	}

	{
		cocos2d::ui::Layout* layout = cocos2d::ui::Layout::create();
		layout->setLayoutType(cocos2d::ui::LayoutType::ABSOLUTE);
		layout->setContentSize(Size(mTabControl->getContentSize().width, mTabControl->getContentSize().height));

		{
			{
				auto text = cocos2d::ui::Text::create("Name", "fonts / Marker Felt.ttf", 10);
				text->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
				text->setColor(Color3B::BLACK);
				text->setPosition(Vec2(0, 460));
				text->setContentSize(Size(260, 20));
				layout->addChild(text);
			}

			{
				auto text = cocos2d::ui::Text::create("Latenz", "fonts / Marker Felt.ttf", 10);
				text->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
				text->setColor(Color3B::BLACK);
				text->setPosition(Vec2(260, 460));
				text->setContentSize(Size(50, 20));
				layout->addChild(text);
			}

			{
				auto text = cocos2d::ui::Text::create("Map", "fonts / Marker Felt.ttf", 10);
				text->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
				text->setColor(Color3B::BLACK);
				text->setPosition(Vec2(310, 460));
				text->setContentSize(Size(100, 20));
				layout->addChild(text);
			}

			{
				auto text = cocos2d::ui::Text::create("Modus", "fonts / Marker Felt.ttf", 10);
				text->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
				text->setColor(Color3B::BLACK);
				text->setPosition(Vec2(410, 460));
				text->setContentSize(Size(100, 20));
				layout->addChild(text);
			}

			{
				auto text = cocos2d::ui::Text::create("Num", "fonts / Marker Felt.ttf", 10);
				text->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
				text->setColor(Color3B::BLACK);
				text->setPosition(Vec2(510, 460));
				text->setContentSize(Size(20, 20));
				layout->addChild(text);
			}

			{
				auto text = cocos2d::ui::Text::create("Max", "fonts / Marker Felt.ttf", 10);
				text->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
				text->setColor(Color3B::BLACK);
				text->setPosition(Vec2(530, 460));
				text->setContentSize(Size(20, 20));
				layout->addChild(text);
			}
		}

		mLanListView = cocos2d::ui::ListView::create();
		mLanListView->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
		mLanListView->setPosition(Point(0, 0));
		mLanListView->setContentSize(Size(600, 450.f));
		mLanListView->setTouchEnabled(false);
		//mLanListView->setClippingEnabled(true);
		mLanListView->setBackGroundColorType(cocos2d::ui::Layout::BackGroundColorType::SOLID);
		mLanListView->setBackGroundColor(cocos2d::Color3B(128, 128, 128));
		mLanListView->setBackGroundColorOpacity(140);
		mLanListView->addEventListener((ui::ListView::ccListViewCallback)CC_CALLBACK_2(ServerBrowser::onSelectedItemEvent, this));
		//mLanListView->addEventListener((ui::ListView::ccScrollViewCallback)CC_CALLBACK_2(UIListViewTest_Vertical::selectedItemEventScrollView, this));
		layout->addChild(mLanListView);

		auto tabLanHeader = cocos2d::ui::TabHeader::create();
		tabLanHeader->setTitleText("LAN");
		tabLanHeader->setTitleFontSize(30.f);
		tabLanHeader->setColor(Color3B::BLACK);
		mTabControl->insertTab(1, tabLanHeader, layout);
	}

	mTabControl->setSelectTab(0);


	float tabBottom = mTabControl->getPosition().y - mTabControl->getContentSize().height / 2.f;

	auto modeLabel = ui::Text::create("Mode:", "fonts/Marker Felt.ttf", 20);
	modeLabel->setAnchorPoint(Vec2::ANCHOR_TOP_RIGHT);
	modeLabel->setColor(Color3B::BLACK);
	modeLabel->setPosition(cocos2d::Vec2(mTabControl->getPosition().x - 200.f, tabBottom - 10.f));
	addChild(modeLabel);

	auto modeTextField = cocos2d::ui::TextField::create("Mode name", "fonts/Marker Felt.ttf", 20);
	modeTextField->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	modeTextField->setTextAreaSize(Size(240, 20));
	modeTextField->setTextColor(Color4B::BLACK);
	modeTextField->setTextHorizontalAlignment(TextHAlignment::LEFT);
	modeTextField->setTextVerticalAlignment(TextVAlignment::CENTER);
	modeTextField->setPosition(Vec2(mTabControl->getPosition().x - 200.f, tabBottom - 10.f));
	modeTextField->addEventListener([=](Ref* sender, ui::TextField::EventType type)
	{
		std::string str;
		switch (type)
		{
		case ui::TextField::EventType::INSERT_TEXT:
		{
			ui::TextField* textField = static_cast<ui::TextField*>(sender);
			str = textField->getString();
			mFilterServer.modeName = str;
			filterServers();
			break;
		}
		case ui::TextField::EventType::DELETE_BACKWARD:
		{
			ui::TextField* textField = static_cast<ui::TextField*>(sender);
			str = textField->getString();
			mFilterServer.modeName = str;
			filterServers();
			break;
		}
		}
	});
	addChild(modeTextField);
	

	auto mapLabel = ui::Text::create("Map:", "fonts/Marker Felt.ttf", 20);
	mapLabel->setAnchorPoint(Vec2::ANCHOR_TOP_RIGHT);
	mapLabel->setColor(Color3B::BLACK);
	mapLabel->setPosition(cocos2d::Vec2(mTabControl->getPosition().x - 200.f, tabBottom - 40.f));
	addChild(mapLabel);

	auto mapTextField = cocos2d::ui::TextField::create("Map name", "fonts/Marker Felt.ttf", 20);
	mapTextField->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	mapTextField->setTextAreaSize(Size(240, 20));
	mapTextField->setTextColor(Color4B::BLACK);
	mapTextField->setTextHorizontalAlignment(TextHAlignment::LEFT);
	mapTextField->setTextVerticalAlignment(TextVAlignment::CENTER);
	mapTextField->setPosition(Vec2(mTabControl->getPosition().x - 200.f, tabBottom - 40.f));
	mapTextField->addEventListener([=](Ref* sender, ui::TextField::EventType type)
	{
		std::string str;
		switch (type)
		{
		case ui::TextField::EventType::INSERT_TEXT:
		{
			ui::TextField* textField = static_cast<ui::TextField*>(sender);
			str = textField->getString();
			mFilterServer.mapName = str;
			filterServers();
			break;
		}
		case ui::TextField::EventType::DELETE_BACKWARD:
		{
			ui::TextField* textField = static_cast<ui::TextField*>(sender);
			str = textField->getString();
			mFilterServer.mapName = str;
			filterServers();
			break;
		}
		}
	});
	addChild(mapTextField);


	auto latencyLabel = ui::Text::create("Latency:", "fonts/Marker Felt.ttf", 20);
	latencyLabel->setAnchorPoint(Vec2::ANCHOR_TOP_RIGHT);
	latencyLabel->setColor(Color3B::BLACK);
	latencyLabel->setPosition(cocos2d::Vec2(mTabControl->getPosition().x - 200.f, tabBottom - 70.f));
	addChild(latencyLabel);

	auto latencyTextField = cocos2d::ui::TextField::create("Latency", "fonts/Marker Felt.ttf", 20);
	latencyTextField->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	latencyTextField->setTextAreaSize(Size(240, 20));
	latencyTextField->setTextColor(Color4B::BLACK);
	latencyTextField->setTextHorizontalAlignment(TextHAlignment::LEFT);
	latencyTextField->setTextVerticalAlignment(TextVAlignment::CENTER);
	latencyTextField->setPosition(Vec2(mTabControl->getPosition().x - 200.f, tabBottom - 70.f));
	latencyTextField->addEventListener([=](Ref* sender, ui::TextField::EventType type)
	{
		switch (type)
		{
		case ui::TextField::EventType::INSERT_TEXT:
		{
			ui::TextField* textField = static_cast<ui::TextField*>(sender);
			std::stringstream ss(textField->getString());
			ss >> mFilterServer.maxLatency;

			std::stringstream ss2;
			ss2 << mFilterServer.maxLatency;
			textField->setString(ss2.str());

			std::string s = textField->getString();

			filterServers();
			break;
		}
		case ui::TextField::EventType::DELETE_BACKWARD:
		{
			ui::TextField* textField = static_cast<ui::TextField*>(sender);
			std::stringstream ss(textField->getString());
			ss >> mFilterServer.maxLatency;
			if (ss.fail())
				mFilterServer.maxLatency = 0;
			filterServers();
			break;
		}
		}
	});
	addChild(latencyTextField);


	ui::CheckBox* serverNotFullCheckBox = ui::CheckBox::create("ui/check_box_normal.png",
		"ui/check_box_normal_press.png",
		"ui/check_box_active.png",
		"ui/check_box_normal_disable.png",
		"ui/check_box_active_disable.png");
	serverNotFullCheckBox->setTouchEnabled(true);
	serverNotFullCheckBox->setAnchorPoint(Vec2::ANCHOR_TOP_RIGHT);
	serverNotFullCheckBox->setPosition(cocos2d::Vec2(mTabControl->getPosition().x - 0.f, tabBottom - 10.f));
	serverNotFullCheckBox->setSelected(mFilterServer.isNotFull);
	serverNotFullCheckBox->addEventListener([=](Ref* sender, ui::CheckBox::EventType type) {
		switch (type)
		{
		case ui::CheckBox::EventType::SELECTED:
			mFilterServer.isNotFull = true;
			filterServers();
			break;
		case ui::CheckBox::EventType::UNSELECTED:
			mFilterServer.isNotFull = false;
			filterServers();
			break;
		};
	});
	addChild(serverNotFullCheckBox);

	auto serverNotFullLabel = ui::Text::create("Server not full", "fonts/Marker Felt.ttf", 20);
	serverNotFullLabel->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	serverNotFullLabel->setColor(Color3B::BLACK);
	serverNotFullLabel->setPosition(cocos2d::Vec2(mTabControl->getPosition().x - 0.f, tabBottom - 10.f));
	addChild(serverNotFullLabel);

	ui::CheckBox* hasUsersPlayingCheckBox = ui::CheckBox::create("ui/check_box_normal.png",
		"ui/check_box_normal_press.png",
		"ui/check_box_active.png",
		"ui/check_box_normal_disable.png",
		"ui/check_box_active_disable.png");
	hasUsersPlayingCheckBox->setTouchEnabled(true);
	hasUsersPlayingCheckBox->setAnchorPoint(Vec2::ANCHOR_TOP_RIGHT);
	hasUsersPlayingCheckBox->setPosition(cocos2d::Vec2(mTabControl->getPosition().x - 0.f, tabBottom - 40.f));
	hasUsersPlayingCheckBox->setSelected(mFilterServer.hasPlayers);
	hasUsersPlayingCheckBox->addEventListener([=](Ref* sender, ui::CheckBox::EventType type) {
		switch (type)
		{
		case ui::CheckBox::EventType::SELECTED:
			mFilterServer.hasPlayers = true;
			filterServers();
			break;
		case ui::CheckBox::EventType::UNSELECTED:
			mFilterServer.hasPlayers = false;
			filterServers();
			break;
		};
	});
	addChild(hasUsersPlayingCheckBox);

	auto hasUsersPlayingLabel = ui::Text::create("Has users playing", "fonts/Marker Felt.ttf", 20);
	hasUsersPlayingLabel->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	hasUsersPlayingLabel->setColor(Color3B::BLACK);
	hasUsersPlayingLabel->setPosition(cocos2d::Vec2(mTabControl->getPosition().x - 0.f, tabBottom - 40.f));
	addChild(hasUsersPlayingLabel);

	ui::CheckBox* noPasswordCheckBox = ui::CheckBox::create("ui/check_box_normal.png",
		"ui/check_box_normal_press.png",
		"ui/check_box_active.png",
		"ui/check_box_normal_disable.png",
		"ui/check_box_active_disable.png");
	noPasswordCheckBox->setTouchEnabled(true);
	noPasswordCheckBox->setAnchorPoint(Vec2::ANCHOR_TOP_RIGHT);
	noPasswordCheckBox->setPosition(cocos2d::Vec2(mTabControl->getPosition().x - 0.f, tabBottom - 70.f));
	noPasswordCheckBox->setSelected(mFilterServer.noPassword);
	noPasswordCheckBox->addEventListener([=](Ref* sender, ui::CheckBox::EventType type) {
		switch (type)
		{
		case ui::CheckBox::EventType::SELECTED:
			mFilterServer.noPassword = true;
			filterServers();
			break;
		case ui::CheckBox::EventType::UNSELECTED:
			mFilterServer.noPassword = false;
			filterServers();
			break;
		};
	});
	addChild(noPasswordCheckBox);

	auto noPasswordLabel = ui::Text::create("Is not password protected", "fonts/Marker Felt.ttf", 20);
	noPasswordLabel->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	noPasswordLabel->setColor(Color3B::BLACK);
	noPasswordLabel->setPosition(cocos2d::Vec2(mTabControl->getPosition().x - 0.f, tabBottom - 70.f));
	addChild(noPasswordLabel);

	auto refreshButton = cocos2d::MenuItemFont::create("Refresh", CC_CALLBACK_1(ServerBrowser::onRefreshServerList, this));
	refreshButton->setFontSizeObj(20);
	refreshButton->setColor(Color3B::BLACK);
	refreshButton->setAnchorPoint(Vec2::ANCHOR_TOP_RIGHT);
	refreshButton->setPosition(Vec2(mTabControl->getPosition().x + mTabControl->getContentSize().width / 2.f, tabBottom - 0.f));
	addControlButton(refreshButton);

	auto connecthButton = cocos2d::MenuItemFont::create("Connect", CC_CALLBACK_1(ServerBrowser::onConnectToServer, this));
	connecthButton->setFontSizeObj(20);
	connecthButton->setColor(Color3B::BLACK);
	connecthButton->setAnchorPoint(Vec2::ANCHOR_TOP_RIGHT);
	connecthButton->setPosition(Vec2(mTabControl->getPosition().x + mTabControl->getContentSize().width / 2.f, tabBottom - 120.f));
	connecthButton->selected();
	addControlButton(connecthButton);

	auto serverAddressLabel = ui::Text::create("Server address:", "fonts/Marker Felt.ttf", 20);
	serverAddressLabel->setAnchorPoint(Vec2::ANCHOR_TOP_RIGHT);
	serverAddressLabel->setColor(Color3B::BLACK);
	serverAddressLabel->setPosition(cocos2d::Vec2(mTabControl->getPosition().x - 0.f, tabBottom - 120.f));
	addChild(serverAddressLabel);

	mServerAdressTextField = cocos2d::ui::TextField::create("address", "fonts/Marker Felt.ttf", 20);
	mServerAdressTextField->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	mServerAdressTextField->setTextColor(Color4B::BLACK);
	mServerAdressTextField->setTextAreaSize(Size(240, 20));
	mServerAdressTextField->setTextHorizontalAlignment(TextHAlignment::LEFT);
	mServerAdressTextField->setTextVerticalAlignment(TextVAlignment::CENTER);
	mServerAdressTextField->setPosition(Vec2(mTabControl->getPosition().x + 10.f, tabBottom - 120.f));
	addChild(mServerAdressTextField);


	auto backButton = cocos2d::MenuItemFont::create("Back", CC_CALLBACK_1(ServerBrowser::onBack, this));
	backButton->setFontSizeObj(50);
	backButton->setColor(Color3B::BLACK);
	backButton->setPosition(cocos2d::Vec2(winSize.width / 2.f, 50.f));
	addControlButton(backButton);

	auto menu = cocos2d::Menu::create(refreshButton, connecthButton, backButton, nullptr);
	menu->setPosition(cocos2d::Vec2(0, 0));
	addChild(menu);

	return true;
}

void ServerBrowser::init(GameSystem* game)
{
	MenuBase::init(game);

	scheduleUpdate();
}

void ServerBrowser::createServerLayout(ServerInfo* serverInfo)
{
	cocos2d::ui::Layout* layout = cocos2d::ui::Layout::create();
	layout->setTouchEnabled(true);
	layout->setLayoutType(cocos2d::ui::LayoutType::ABSOLUTE);
	layout->setContentSize(Size(mTabControl->getContentSize().width, 22));

	auto serverNameText = cocos2d::ui::Text::create(serverInfo->name.empty() ? "unknown" : serverInfo->name, "fonts / Marker Felt.ttf", 20);
	serverNameText->setColor(cocos2d::Color3B::BLACK);
	serverNameText->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
	serverNameText->setPosition(Vec2(0, 0));
	serverNameText->setContentSize(Size(260, 22));
	layout->addChild(serverNameText);

	std::stringstream ss;
	ss << serverInfo->latency;
	auto pingText = cocos2d::ui::Text::create(ss.str(), "fonts / Marker Felt.ttf", 20);
	pingText->setColor(cocos2d::Color3B::BLACK);
	pingText->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
	pingText->setPosition(Vec2(260, 0));
	pingText->setContentSize(Size(50, 22));
	layout->addChild(pingText);

	auto mapText = cocos2d::ui::Text::create(serverInfo->mapName, "fonts / Marker Felt.ttf", 20);
	mapText->setColor(cocos2d::Color3B::BLACK);
	mapText->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
	mapText->setPosition(Vec2(310, 0));
	mapText->setContentSize(Size(100, 22));
	layout->addChild(mapText);

	auto modeText = cocos2d::ui::Text::create(serverInfo->inLobby ? "Lobby" : serverInfo->modeName, "fonts / Marker Felt.ttf", 20);
	modeText->setColor(cocos2d::Color3B::BLACK);
	modeText->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
	modeText->setPosition(Vec2(410, 0));
	modeText->setContentSize(Size(100, 22));
	layout->addChild(modeText);

	ss.str("");
	ss.clear();
	ss << serverInfo->numPlayers;
	auto numPlayerText = cocos2d::ui::Text::create(ss.str(), "fonts / Marker Felt.ttf", 20);
	numPlayerText->setColor(cocos2d::Color3B::BLACK);
	numPlayerText->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
	numPlayerText->setPosition(Vec2(510, 0));
	numPlayerText->setContentSize(Size(20, 22));
	layout->addChild(numPlayerText);

	ss.str("");
	ss.clear();
	ss << serverInfo->maxPlayers;
	auto maxPlayerText = cocos2d::ui::Text::create(ss.str(), "fonts / Marker Felt.ttf", 20);
	maxPlayerText->setColor(cocos2d::Color3B::BLACK);
	maxPlayerText->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
	maxPlayerText->setPosition(Vec2(530, 0));
	maxPlayerText->setContentSize(Size(20, 22));
	layout->addChild(maxPlayerText);

	auto joinButton = cocos2d::ui::Button::create();
	joinButton->setTitleColor(cocos2d::Color3B::BLACK);
	joinButton->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
	joinButton->setPosition(Vec2(550, 0));
	joinButton->setTitleText("Join");
	joinButton->setTitleFontSize(20);
	joinButton->setContentSize(Size(50, 22));
	joinButton->addClickEventListener(CC_CALLBACK_1(ServerBrowser::onJoinServer, this));
	layout->addChild(joinButton);

	serverInfo->layout = layout;
	serverInfo->joinButton = joinButton;
	serverInfo->latenceText = pingText;
	serverInfo->mapText = mapText;
	serverInfo->modeText = modeText;
	serverInfo->numPlayerText = numPlayerText;
	serverInfo->maxPlayerText = maxPlayerText;

	if (serverInfo->searchType == ST_INTERNET)
	{
		serverInfo->tabIndex = mInternetListView->getItems().size();
		mInternetListView->pushBackCustomItem(layout);
	}
	else
	{
		serverInfo->tabIndex = mLanListView->getItems().size();
		mLanListView->pushBackCustomItem(layout);
	}
}

void ServerBrowser::updateServerLayout(ServerInfo* serverInfo)
{
	{
		std::stringstream ss;
		ss << serverInfo->latency;
		serverInfo->latenceText->setString(ss.str());
	}
	serverInfo->mapText->setString(serverInfo->mapName);
	serverInfo->modeText->setString(serverInfo->inLobby ? "Lobby" : serverInfo->modeName);
	{
		std::stringstream ss;
		ss << serverInfo->numPlayers;
		serverInfo->numPlayerText->setString(ss.str());
	}
	{
		std::stringstream ss;
		ss << serverInfo->maxPlayers;
		serverInfo->maxPlayerText->setString(ss.str());
	}
}

void ServerBrowser::onRefreshServerList(cocos2d::Ref* sender)
{
	if (mTabControl->getSelectedTabIndex() == 0)
	{
		auto it = mServerInfos.begin();
		while (it != mServerInfos.end())
		{
			if ((*it).searchType == ST_INTERNET)
				it = mServerInfos.erase(it);
			else
				++it;
		}
		mInternetListView->removeAllItems();

		mGame->mNetwork->mMasterClient->requestServerList();
	}
	else if (mTabControl->getSelectedTabIndex() == 1)
	{
		auto it = mServerInfos.begin();
		while (it != mServerInfos.end())
		{
			if ((*it).searchType == ST_LAN)
				it = mServerInfos.erase(it);
			else
				++it;
		}
		mLanListView->removeAllItems();

		mGame->mNetwork->mSearchLanHost->startSearch();
	}
}

void ServerBrowser::onJoinServer(cocos2d::Ref* sender)
{
	if (mServerInfoSocket != ENET_SOCKET_NULL)
	{
		enet_socket_destroy(mServerInfoSocket);
		mServerInfoSocket = ENET_SOCKET_NULL;
	}

	ServerInfo* serverInfo = nullptr;
	for (auto& server : mServerInfos)
	{
		if (server.joinButton == sender)
		{
			serverInfo = &server;
			break;
		}
	}

	if (serverInfo)
	{
		char hn[260];
		if (enet_address_get_host_ip(&serverInfo->address, hn, sizeof(hn)) == 0)
		{
			if (mGame->mNetwork->connect(hn, serverInfo->address.port))
			{
				Director::getInstance()->pushScene(LobbyMenu::createScene(mGame));
			}
		}
	}
}

void ServerBrowser::onConnectToServer(cocos2d::Ref* sender)
{
	if (mGame->mNetwork->isClient())
		return;

	std::uint16_t port = 0;

	std::string address = mServerAdressTextField->getString();

	size_t pos = address.find_first_of(':');
	std::string ip = address.substr(0, pos);

	if (pos != address.npos)
	{
		std::string portStr = address.substr(pos + 1);
		std::stringstream ss(portStr);
		ss >> port;

		if (!port)
		{
			port = NET_DEFAULT_PORT;
		}
	}
	else
	{
		port = NET_DEFAULT_PORT;
	}

	if (mGame->mNetwork->connect(ip, port))
	{
		Director::getInstance()->pushScene(LobbyMenu::createScene(mGame));
	}
}

void ServerBrowser::onSelectedItemEvent(Ref* sender, cocos2d::ui::ListView::EventType type)
{
	if (type == cocos2d::ui::ListView::EventType::ON_SELECTED_ITEM_START)
	{
		ui::ListView* listView = static_cast<ui::ListView*>(sender);
		auto item = listView->getItem(listView->getCurSelectedIndex());

		ServerInfo* serverInfo = nullptr;
		for (auto& server : mServerInfos)
		{
			if (server.layout == item)
			{
				serverInfo = &server;
				break;
			}
		}

		if (serverInfo)
		{
			char hn[260];
			if (enet_address_get_host_ip(&serverInfo->address, hn, sizeof(hn)) == 0)
			{
				std::string hostname = hn;
				std::stringstream ss;
				ss << serverInfo->address.port;

				std::string address = hostname + ":" + ss.str();

				mServerAdressTextField->setString("test");
			}
		}
	}
}

void ServerBrowser::onBack(cocos2d::Ref* sender)
{
	Director::getInstance()->popScene();
}

bool ServerBrowser::updateServerListFromMaster()
{
	mGame->mNetwork->mMasterClient->requestServerList();

	return true;
}

void ServerBrowser::onServerListReady(MasterClient* masterClient)
{
	for (auto& server : masterClient->mGameServers)
	{
		ServerInfo info;
		info.address.host = server.address.host;
		info.address.port = server.address.port;
		info.infoPort = server.infoPort;
		info.searchType = ST_INTERNET;
		mServerInfos.push_back(info);
	}
}

bool ServerBrowser::pingServers()
{
	if (mServerInfos.size() == 0)
		return true;

	if (mServerInfoSocket == ENET_SOCKET_NULL)
	{
		mServerInfoSocket = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
		if (mServerInfoSocket == ENET_SOCKET_NULL)
			return false;

		enet_socket_set_option(mServerInfoSocket, ENET_SOCKOPT_NONBLOCK, 1);
		enet_socket_set_option(mServerInfoSocket, ENET_SOCKOPT_BROADCAST, 1);
	}

	ENetBuffer buf;
	std::uint8_t data[] = {0};
	buf.data = data;
	buf.dataLength = sizeof(data);


	std::uint32_t max = std::min(mServerInfos.size(), mMaxPingServers);
	for (size_t i = 0; i < max; ++i)
	{
		auto& server = mServerInfos[mLastPingServer++];
		if (mLastPingServer >= mServerInfos.size())
			mLastPingServer = 0;
		
		ENetAddress infoAddress;
		infoAddress.host = server.address.host;
		infoAddress.port = server.infoPort;

		int sent = enet_socket_send(mServerInfoSocket, &infoAddress, &buf, 1);
		if (sent > 0)
		{
			server.requestTime = enet_time_get();
		}
	}

	mLastInfo = enet_time_get();

	return true;
}

void ServerBrowser::onLanServerFound(std::uint32_t host, std::uint16_t port)
{
	for (size_t i = 0; i < mServerInfos.size(); ++i)
	{
		if (mServerInfos[i].address.host == host && mServerInfos[i].address.port == port)
			return;
	}

	ServerInfo info;
	info.address.host = host;
	info.address.port = port;
	info.infoPort = port + 1;
	info.searchType = ST_LAN;
	mServerInfos.push_back(info);

	mLastInfo = 0;
}

void ServerBrowser::filterServers()
{
	for (size_t i = 0; i < mServerInfos.size(); ++i)
	{
		mServerInfos[i].isFiltered = false;

		if (mFilterServer.hasPlayers)
		{
			if (mServerInfos[i].numPlayers <= 1)
				mServerInfos[i].isFiltered = true;
		}
		if (mFilterServer.isNotFull)
		{
			if (mServerInfos[i].numPlayers == mServerInfos[i].maxPlayers)
				mServerInfos[i].isFiltered = true;
		}
		if (mFilterServer.noPassword)
		{
			if (mServerInfos[i].password)
				mServerInfos[i].isFiltered = true;
		}
		if (mFilterServer.maxLatency != 0)
		{
			if (mServerInfos[i].latency > mFilterServer.maxLatency)
				mServerInfos[i].isFiltered = true;
		}
		if (!mFilterServer.mapName.empty())
		{
			std::string lowerMapName = mServerInfos[i].mapName;
			std::string lowerFilterName = mFilterServer.mapName;
			std::transform(lowerMapName.begin(), lowerMapName.end(), lowerMapName.begin(), ::tolower);
			std::transform(lowerFilterName.begin(), lowerFilterName.end(), lowerFilterName.begin(), ::tolower);

			if (lowerMapName.find(lowerFilterName) == std::string::npos)
				mServerInfos[i].isFiltered = true;
		}
		if (!mFilterServer.modeName.empty())
		{
			std::string lowerModeName = mServerInfos[i].modeName;
			std::string lowerFilterName = mFilterServer.modeName;
			std::transform(lowerModeName.begin(), lowerModeName.end(), lowerModeName.begin(), ::tolower);
			std::transform(lowerFilterName.begin(), lowerFilterName.end(), lowerFilterName.begin(), ::tolower);

			if (lowerModeName.find(lowerFilterName) == std::string::npos)
				mServerInfos[i].isFiltered = true;
		}
	}

	mInternetListView->removeAllItems();
	mLanListView->removeAllItems();
	for (size_t i = 0; i < mServerInfos.size(); ++i)
	{
		if (!mServerInfos[i].isFiltered)
		{
			if (mServerInfos[i].searchType == ST_INTERNET)
				mInternetListView->pushBackCustomItem(mServerInfos[i].layout);
			else if (mServerInfos[i].searchType == ST_LAN)
				mLanListView->pushBackCustomItem(mServerInfos[i].layout);
		}
	}
}

void ServerBrowser::sortServers()
{
	std::sort(mServerInfos.begin(), mServerInfos.end(), [](const ServerInfo& s1, const ServerInfo& s2) {
		return s1.latency < s2.latency;
	});

	mInternetListView->removeAllItems();
	mLanListView->removeAllItems();
	for (size_t i = 0; i < mServerInfos.size(); ++i)
	{
		if (!mServerInfos[i].isFiltered)
		{
			if (mServerInfos[i].searchType == ST_INTERNET)
				mInternetListView->pushBackCustomItem(mServerInfos[i].layout);
			else if (mServerInfos[i].searchType == ST_LAN)
				mLanListView->pushBackCustomItem(mServerInfos[i].layout);
		}
	}
}

void ServerBrowser::update(float deltaTime)
{
	if (mGame->mNetwork->mSearchLanHost)
	{
		mGame->mNetwork->mSearchLanHost->update(deltaTime);
	}

	if (mServerInfos.size() > 0)
	{
		std::uint32_t max = std::min(mServerInfos.size(), mMaxPingServers);
		if (!mLastInfo || ENET_TIME_DIFFERENCE(enet_time_get(), mLastInfo) >= mServerPingRate / std::max(1, int(mServerInfos.size() + max - 1)) / max)
		{
			pingServers();
		}
	}

	// checkpongs
	if (mServerInfoSocket != ENET_SOCKET_NULL)
	{
		ENetBuffer buf;
		ENetAddress addr;

		std::uint8_t data[240];

		buf.data = data;
		buf.dataLength = sizeof(data);

		int len = enet_socket_receive(mServerInfoSocket, &addr, &buf, 1);
		if (len > 0)
		{
			NetMessage msg;
			msg.write(buf.data, len);

			ServerInfo* serverInfo = nullptr;
			for (auto& server : mServerInfos)
			{
				if (server.address.host == addr.host && server.infoPort == addr.port)
				{
					serverInfo = &server;
				}
			}

			if (serverInfo && !serverInfo->isFiltered)
			{
				std::uint8_t numPlayers = 0;
				std::uint8_t maxPlayers = 0;

				msg.readString(serverInfo->name);
				msg.readBool(serverInfo->inLobby);
				msg.readString(serverInfo->mapName);
				msg.readString(serverInfo->modeName);
				msg.readUByte(numPlayers);
				msg.readUByte(maxPlayers);
				msg.readBool(serverInfo->password);

				serverInfo->numPlayers = numPlayers;
				serverInfo->maxPlayers = maxPlayers;

				serverInfo->latency = std::min(int((enet_time_get() - serverInfo->requestTime)), 999);

				if (!serverInfo->layout)
				{
					createServerLayout(serverInfo);
				}
				else
				{
					updateServerLayout(serverInfo);
				}
			}
		}
	}
}

void ServerBrowser::onEnter()
{
	MenuBase::onEnter();

	if (mInternetListView)
		mInternetListView->removeAllItems();
	if (mLanListView)
		mLanListView->removeAllItems();
	mServerInfos.clear();

	mGame->mNetwork->mSearchLanHost = std::make_unique<SearchLanHost>();
	mGame->mNetwork->mSearchLanHost->mOnFound = std::bind(&ServerBrowser::onLanServerFound, this, std::placeholders::_1, std::placeholders::_2);
	mGame->mNetwork->mSearchLanHost->startSearch();

	mGame->mNetwork->mMasterClient = std::make_unique<MasterClient>(mGame->mNetwork->mMasterServerIp, mGame->mNetwork->mMasterServerPort);
	mGame->mNetwork->mMasterClient->mOnServerListReady = std::bind(&ServerBrowser::onServerListReady, this, std::placeholders::_1);

	updateServerListFromMaster();
}

void ServerBrowser::onExit()
{
	MenuBase::onExit();

	mGame->mNetwork->mSearchLanHost.reset();
	mGame->mNetwork->mMasterClient.reset();

	if (mServerInfoSocket != ENET_SOCKET_NULL)
	{
		enet_socket_destroy(mServerInfoSocket);
		mServerInfoSocket = ENET_SOCKET_NULL;
	}
}