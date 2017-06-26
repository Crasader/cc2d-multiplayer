#include "LobbyMenu.h"

#include "GameSystem.h"
#include "NetworkManager.h"
#include "MapSelectorMenu.h"
#include "Lobby.h"
#include "GameMode.h"

USING_NS_CC;

LobbyMenu::LobbyMenu()
{
}

LobbyMenu::~LobbyMenu()
{

}

Scene* LobbyMenu::createScene(GameSystem* game)
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = LobbyMenu::create();
	layer->mGame = game;
	layer->mLobby = game->mNetwork->mLobby.get();

	layer->initMenu();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

bool LobbyMenu::init()
{
	if (!Layer::init())
	{
		return false;
	}

	scheduleUpdate();

	return true;
}

void LobbyMenu::initMenu()
{
	mGame->mLobby = mLobby;
	mLobby->removeAllUsers();

	auto winSize = Director::getInstance()->getWinSize();

	cocos2d::Vector<cocos2d::MenuItem*> items;

	auto backgroundSprite = Sprite::create("Background.png");
	float scalingX = winSize.width / backgroundSprite->getBoundingBox().size.width;
	float scalingY = winSize.height / backgroundSprite->getBoundingBox().size.height;
	backgroundSprite->setScaleX(scalingX);
	backgroundSprite->setScaleY(scalingY);
	backgroundSprite->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
	addChild(backgroundSprite, -1);

	auto textLabel = ui::Text::create("Lobby", "fonts/Marker Felt.ttf", 120);
	textLabel->setColor(Color3B::BLACK);
	textLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_TOP);
	textLabel->setPosition(cocos2d::Vec2(winSize.width / 2.f, winSize.height - 50.f));
	addChild(textLabel);

	mMapText = ui::Text::create(mGame->getNextMap(), "fonts/Marker Felt.ttf", 50);
	mMapText->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	mMapText->setColor(Color3B::BLACK);
	mMapText->setPosition(cocos2d::Vec2(winSize.width / 2, winSize.height / 2 + 100));
	addChild(mMapText);

	mLobby->mLobbyUsers.resize(mGame->mMaxPlayers);

	for (size_t i = 0; i < NET_MAX_CLIENTS; ++i)
	{
		std::stringstream ss;
		ss << (i + 1);

		auto userSlot = cocos2d::ui::Button::create();
		userSlot->setTitleText("Slot " + ss.str());
		userSlot->setTitleFontSize(20);
		userSlot->setColor(Color3B::BLACK);
		userSlot->setPosition(cocos2d::Vec2(winSize.width / 2.f, winSize.height / 2.f + 60.f - (20.f * i)));
		userSlot->setEnabled(false);

		if (i > mGame->mNetwork->mMaxConnections - 1)
			userSlot->setVisible(false);

		addChild(userSlot);

		mUserSlots.push_back(userSlot);
	}

	if (mGame->mNetwork->isServer())
	{
		auto changeMapButton = cocos2d::MenuItemFont::create("Change Map", CC_CALLBACK_1(LobbyMenu::changeMap, this));
		changeMapButton->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
		changeMapButton->setFontSizeObj(50);
		changeMapButton->setColor(Color3B::BLACK);
		changeMapButton->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
		changeMapButton->setPosition(Vec2(winSize.width / 2.f, winSize.height / 2.f - 60.f));
		addControlButton(changeMapButton);
		items.pushBack(changeMapButton);

		auto roundsLabel = ui::Text::create();
		roundsLabel->setFontSize(50.f);
		roundsLabel->setString("Rounds: ");
		roundsLabel->setColor(Color3B::BLACK);
		roundsLabel->setPosition(cocos2d::Vec2(winSize.width / 2 - 70, winSize.height / 2 - 120));
		addChild(roundsLabel, 0);

		auto minus = cocos2d::MenuItemFont::create("<", CC_CALLBACK_1(LobbyMenu::onMinus, this));
		minus->setFontSizeObj(60);
		minus->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
		minus->setColor(Color3B::BLACK);
		minus->setPosition(winSize.width / 2.f + 40, winSize.height / 2.f - 120);
		addControlButton(minus);
		items.pushBack(minus);

		std::stringstream ss;
		ss << mGame->mTotalRoundCounter;
		mRoundsNumLabel = ui::Text::create();
		mRoundsNumLabel->setFontSize(50.f);
		mRoundsNumLabel->setColor(Color3B::BLACK);
		mRoundsNumLabel->setString(ss.str());
		mRoundsNumLabel->setPosition(cocos2d::Vec2(winSize.width / 2 + 80, winSize.height / 2 - 120));
		addChild(mRoundsNumLabel, 0);

		auto plus = cocos2d::MenuItemFont::create(">", CC_CALLBACK_1(LobbyMenu::onPlus, this));
		plus->setFontSizeObj(60);
		plus->setColor(Color3B::BLACK);
		plus->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
		plus->setPosition(winSize.width / 2.f + 120, winSize.height / 2.f - 120);
		addControlButton(plus);
		items.pushBack(plus);


		auto startServerButton = cocos2d::MenuItemFont::create("Start Game", CC_CALLBACK_1(LobbyMenu::onStartGame, this));
		startServerButton->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
		startServerButton->setFontSizeObj(50);
		startServerButton->setColor(Color3B::BLACK);
		startServerButton->setPosition(Vec2(winSize.width / 2.f, winSize.height / 2.f - 240.f));
		startServerButton->selected();
		addControlButton(startServerButton);
		items.pushBack(startServerButton);

		addUser(NetworkManager::ServerId, mLobby->mNetwork->mPlayerName);
	}

	auto backButton = cocos2d::MenuItemFont::create("Back", CC_CALLBACK_1(LobbyMenu::onBack, this));
	backButton->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	backButton->setFontSizeObj(50);
	backButton->setColor(Color3B::BLACK);
	backButton->setPosition(cocos2d::Vec2(winSize.width / 2.f, winSize.height / 2.f - 300.f));
	addControlButton(backButton);
	items.pushBack(backButton);

	auto menu = cocos2d::Menu::createWithArray(items);
	menu->setPosition(0, 0);
	addChild(menu, 0);

	// Chat
	{
		mChatScrollView = cocos2d::ui::ScrollView::create();
		mChatScrollView->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
		mChatScrollView->setPosition(Point(winSize.width - 300, winSize.height / 2 - 80));
		mChatScrollView->setBackGroundColorType(cocos2d::ui::Layout::BackGroundColorType::SOLID);
		mChatScrollView->setBackGroundColor(cocos2d::Color3B(128, 128, 128));
		mChatScrollView->setBackGroundColorOpacity(140);
		mChatScrollView->setContentSize(Size(240.0f, 200.0f));
		mChatScrollView->setDirection(ui::ScrollView::Direction::VERTICAL);
		mChatScrollView->setClippingEnabled(true);
		mChatScrollView->setLayoutType(ui::Layout::Type::VERTICAL);
		//mChatScrollView->setBounceEnabled(true);
		addChild(mChatScrollView);

		mChatHistory = ui::Text::create();
		mChatHistory->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
		mChatHistory->setFontSize(20);
		mChatHistory->setColor(Color3B::BLACK);
		mChatHistory->setFontName("fonts/Marker Felt.ttf");
		//mChatHistory->ignoreContentAdaptWithSize(false);
		//mChatHistory->setContentSize(Size(240.0f, 250.0f));
		mChatHistory->setTextAreaSize(Size(230.f, 0.f));
		mChatHistory->setTextHorizontalAlignment(TextHAlignment::LEFT);
		//mChatHistory->setTextVerticalAlignment(TextVAlignment::BOTTOM);
		mChatScrollView->addChild(mChatHistory);

		mChatTextField = cocos2d::ui::TextField::create("Input", "fonts/Marker Felt.ttf", 20);
		mChatTextField->setAnchorPoint(Vec2(0, 0));
		mChatTextField->setColor(Color3B::BLACK);
		mChatTextField->ignoreContentAdaptWithSize(false);
		mChatTextField->setContentSize(Size(200, 22));
		mChatTextField->setMaxLength(240);
		mChatTextField->setTextHorizontalAlignment(TextHAlignment::LEFT);
		mChatTextField->setTextVerticalAlignment(TextVAlignment::BOTTOM);
		mChatTextField->setPosition(Vec2(winSize.width - 300, winSize.height / 2.0f - 100));
		addChild(mChatTextField);

		auto sendChatButton = cocos2d::ui::Button::create();
		sendChatButton->setAnchorPoint(Vec2(0, 0));
		sendChatButton->setTitleText("Send");
		sendChatButton->setTitleFontSize(20);
		sendChatButton->setColor(Color3B::BLACK);
		sendChatButton->setPosition(cocos2d::Vec2(mChatTextField->getPosition().x + mChatTextField->getContentSize().width, mChatTextField->getPosition().y));
		sendChatButton->addClickEventListener(CC_CALLBACK_1(LobbyMenu::onSendChat, this));
		addChild(sendChatButton);
	}

	mGame->mNetwork->mOnConnect.add(this, &LobbyMenu::onClientConnect);
	mGame->mNetwork->mOnMessage.add(this, &LobbyMenu::onNetMessage);
	mGame->mNetwork->mOnDisconnect.add(this, &LobbyMenu::onClientDisconnect);
}

void LobbyMenu::addUser(std::uint16_t clientId, const std::string name)
{
	std::uint16_t freeLobbyId = mLobby->addUser(clientId, name);

	addUser(freeLobbyId, clientId, name);
}

void LobbyMenu::addUser(size_t index, std::uint16_t clientId, const std::string name)
{
	mLobby->addUser(index, clientId, name);

	mUserSlots[index]->setTitleText(mLobby->getUser(index)->name);
	mUserSlots[index]->setVisible(true);
}

void LobbyMenu::removeUser(std::uint16_t clientId)
{
	for (size_t i = 0; i < mLobby->mLobbyUsers.size(); ++i)
	{
		if (mLobby->mLobbyUsers[i].isConnected &&  mLobby->mLobbyUsers[i].clientId == clientId)
		{
			removeUserAtIndex(i);
			break;
		}
	}
}

void LobbyMenu::removeUserAtIndex(size_t index)
{
	mLobby->removeUserAtIndex(index);

	std::stringstream ss;
	ss << index;
	mUserSlots[index]->setTitleText("Slot " + ss.str());
	mUserSlots[index]->setVisible(true);
}

void LobbyMenu::changeMap(cocos2d::Ref* sender)
{
	Director::getInstance()->pushScene(MapSelectorMenu::createScene(mGame, this));
}

void LobbyMenu::setMapName(const std::string& map)
{
	mMapText->setString(map);

	mGame->setNextMapName(map);

	sendChangeMap();
}

void LobbyMenu::onMinus(cocos2d::Ref* sender)
{
	if (mGame->mTotalRoundCounter > 1)
	{
		mGame->mTotalRoundCounter--;
		std::stringstream ss;
		ss << mGame->mTotalRoundCounter;
		mRoundsNumLabel->setString(ss.str());
	}
}

void LobbyMenu::onPlus(cocos2d::Ref* sender)
{
	if (mGame->mTotalRoundCounter < 99)
	{
		mGame->mTotalRoundCounter++;
		std::stringstream ss;
		ss << mGame->mTotalRoundCounter;
		mRoundsNumLabel->setString(ss.str());
	}
}

void LobbyMenu::onStartGame(cocos2d::Ref* sender)
{
	if (mGame->mIsGameRunning)
		return;

	mGame->mMapNames.clear();
	mGame->mCurrentRoundCounter = 0;
	mGame->setNextMapName(mMapText->getString());
	mGame->resetUserScores();
	mGame->startGame();
}

void LobbyMenu::onSendChat(cocos2d::Ref* sender)
{
	std::string chatMsg = mChatTextField->getString();
	mChatTextField->setString("");

	addChatText(mGame->mNetwork->mClientId, chatMsg);

	NetMessage msg(NetMessage::MSG_ChatMsg);
	msg.writeUShort(mGame->mNetwork->mClientId);
	msg.writeString(chatMsg);
	mGame->mNetwork->send(msg);
}

void LobbyMenu::addChatText(std::uint16_t clientId, const std::string& text)
{
	Lobby::LobbyUser* user = mLobby->getUserByClientId(clientId);
	if (user)
	{
		std::string chatText = user->name + ": " + text;
		mChatHistory->setString(mChatHistory->getString() + "\n" + chatText);

		Size cs = mChatScrollView->getContentSize();
		Size ics = mChatScrollView->getInnerContainerSize();

		Size size = mChatHistory->getLayoutSize();
		Size tas = mChatHistory->getTextAreaSize();

		mChatScrollView->setInnerContainerSize(Size(240.f, size.height));
		mChatScrollView->scrollToBottom(0, false);
	}
}

void LobbyMenu::onBack(cocos2d::Ref* sender)
{
	mGame->mNetwork->disconnect();
	mGame->mNetwork->mOnConnect.remove(this);
	mGame->mNetwork->mOnDisconnect.remove(this);
	mGame->mNetwork->mOnMessage.remove(this);

	mLobby->removeAllUsers();

	Director::getInstance()->popScene();
}

void LobbyMenu::sendChangeMap()
{
	if (!mGame->mNetwork->isServer())
		return;

	NetMessage msg(NetMessage::MSG_Lobby_ChangeMap, NetworkManager::ServerId);
	msg.writeString(mGame->getNextMap());
	mGame->mNetwork->send(msg);
}

void LobbyMenu::update(float deltaTime)
{
	MenuBase::update(deltaTime);

	if (mGame && mGame->mNetwork)
	{
		mGame->mNetwork->update(deltaTime);
	}
}

void LobbyMenu::onEnter()
{
	MenuBase::onEnter();

	if (!mGame->mNetwork->isConnected())
	{
		cocos2d::Director::getInstance()->popScene();
	}
}

void LobbyMenu::onExit()
{
	MenuBase::onExit();
}

void LobbyMenu::onClientConnect(std::uint32_t clientId)
{
	if (mGame->mNetwork->isServer())
	{
		NetworkManager::Connection* client = mGame->mNetwork->getConnection(clientId);
		if (client)
		{
			addUser(clientId, client->name);

			mLobby->sendLobbyUserData();
			sendChangeMap();

			if (mGame->mGameMode)
				mGame->mGameMode->clientConnected(clientId);
		}
	}
}

void LobbyMenu::onClientDisconnect(std::uint32_t clientId)
{
	if (mGame->mNetwork->isServer())
	{
		removeUser(clientId);

		mLobby->sendLobbyUserData();

		if (mGame->mGameMode)
			mGame->mGameMode->clientDisconnected(clientId);
	}
	else
	{
		if (clientId == NetworkManager::ServerId)
		{
			if (mGame->mIsGameRunning)
			{
				mGame->exitGame();
			}
			onBack(0);
		}
	}
}

void LobbyMenu::onNetMessage(NetMessage& msg)
{
	switch (msg.mMsgId)
	{
	case NetMessage::MSG_ChatMsg:
		{
			std::uint16_t clientId = NetworkManager::InvalidClientId;
			std::string chatText;

			msg.readUShort(clientId);
			msg.readString(chatText);

			addChatText(clientId, chatText);

			if (mGame->mNetwork->isServer())
			{
				NetMessage chatMsg(NetMessage::MSG_ChatMsg);
				chatMsg.writeUShort(clientId);
				chatMsg.writeString(chatText);
				mGame->mNetwork->sendNotTo(clientId, chatMsg);
			}

			break;
		}
	case NetMessage::MSG_Lobby_UserData:
		{
			std::uint16_t numUsers = 0;

			msg.readUShort(numUsers);

			for (size_t i = 0; i < numUsers; ++i)
			{
				bool isConnected = false;
				msg.readBool(isConnected);

				if (isConnected)
				{
					std::uint16_t id;
					std::string userName;

					msg.readUShort(id);
					msg.readString(userName);

					addUser(i, id, userName);
				}
				else
				{
					removeUserAtIndex(i);
				}
			}
			for (size_t i = numUsers; i < mUserSlots.size(); ++i)
			{
				mUserSlots[i]->setVisible(false);
			}

			break;
		}
	case NetMessage::MSG_Lobby_ChangeMap:
		{
			std::string newMapName;
			msg.readString(newMapName);

			setMapName(newMapName);

			break;
		}
	case NetMessage::MSG_Lobby_ExitGame:
		{
			mGame->exitGame();

			break;
		}
	case NetMessage::MSG_MapChange:
		{
			if (mGame->mNetwork->isClient())
			{
				std::uint32_t sequence = 0;
				std::string mapName;
				msg.readUInt(sequence);
				msg.readString(mapName);

				mGame->mNetwork->setCurrentSnapSequence(sequence);

				mGame->setNextMapName(mapName);
				mGame->startGame();
			}

			break;
		}
	}
}