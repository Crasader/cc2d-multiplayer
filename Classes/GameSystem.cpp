#include "GameSystem.h"

#include "Input.h"
#include "Physics.h"
#include "NetworkManager.h"
#include "GameObjectFactory.h"
#include "GameObject.h"
#include "LevelScene.h"
#include "Lobby.h"

#include "GameMode.h"
#include "VolleyMode.h"
#include "StickyBombMode.h"
#include "ScriptGameMode.h"
#include "GameModeFactory.h"

#include "DebugDrawer.h"

#include "Player.h"

#include "ChartPanel.h"

#include "ScriptManager.h"

#include "ScoreMenu.h"

#include "Voting.h"


std::vector<GameSystem::DebugText> GameSystem::mDebugTextVec;

GameSystem::GameSystem()
{
	readConfig();

	mLevelScene = LevelScene::create();
	mLevelScene->mGame = this;

	mInput = std::make_unique<Input>();
	mInput->readConfig(mConfig);
	mLevelScene->registerInput(mInput.get());

	{
		mInput->addAction("menuEnter", cocos2d::EventKeyboard::KeyCode::KEY_KP_ENTER, "menu")
			.addButton(SDL_CONTROLLER_BUTTON_A);

		mInput->addAction("menuBack", cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE, "menu")
			.addButton(SDL_CONTROLLER_BUTTON_B);

		mInput->addAction("menuUp", cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW, "menu")
			.addButton(SDL_CONTROLLER_BUTTON_DPAD_UP);

		mInput->addAction("menuDown", cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW, "menu")
			.addButton(SDL_CONTROLLER_BUTTON_DPAD_DOWN);

		mInput->addAction("exitGame", cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE, "menu")
			.addBind(IE_Pressed, std::bind(&GameSystem::onExitGame, this), this);

		mInput->addAction("toggleDebug", cocos2d::EventKeyboard::KeyCode::KEY_F1, "menu")
			.addBind(IE_Pressed, std::bind(&GameSystem::toggleDebug, this), this);

		mInput->addAction("vote", cocos2d::EventKeyboard::KeyCode::KEY_V, "menu");
		mInput->addAction("voteCancel", cocos2d::EventKeyboard::KeyCode::KEY_0, "menu");
		mInput->addAction("voteOption1", cocos2d::EventKeyboard::KeyCode::KEY_1, "menu");
		mInput->addAction("voteOption2", cocos2d::EventKeyboard::KeyCode::KEY_2, "menu");
		mInput->addAction("voteOption3", cocos2d::EventKeyboard::KeyCode::KEY_3, "menu");
		mInput->addAction("voteOption4", cocos2d::EventKeyboard::KeyCode::KEY_4, "menu");
		mInput->addAction("voteOption5", cocos2d::EventKeyboard::KeyCode::KEY_5, "menu");
		mInput->addAction("voteOption6", cocos2d::EventKeyboard::KeyCode::KEY_6, "menu");
		mInput->addAction("voteOption7", cocos2d::EventKeyboard::KeyCode::KEY_7, "menu");
		mInput->addAction("voteOption8", cocos2d::EventKeyboard::KeyCode::KEY_8, "menu");
		mInput->addAction("voteOption9", cocos2d::EventKeyboard::KeyCode::KEY_9, "menu");

		mInput->addAction("specNext", cocos2d::EventKeyboard::KeyCode::KEY_SPACE, "game")
			.addButton(SDL_CONTROLLER_BUTTON_A);

		mInput->addAction("jump", cocos2d::EventKeyboard::KeyCode::KEY_SPACE, "game")
			.addKey(cocos2d::EventKeyboard::KeyCode::KEY_W)
			.addButton(SDL_CONTROLLER_BUTTON_A);

		mInput->addAxis("right", false, "game")
			.addProp(SDL_CONTROLLER_AXIS_LEFTX, 1.0f)
			.addProp(SDL_CONTROLLER_BUTTON_DPAD_RIGHT, 1.0f)
			.addProp(SDL_CONTROLLER_BUTTON_DPAD_LEFT, -1.0f)
			.addProp(cocos2d::EventKeyboard::KeyCode::KEY_D, 1.0f)
			.addProp(cocos2d::EventKeyboard::KeyCode::KEY_A, -1.0f);
		mInput->addAxis("up", false, "game")
			.addProp(SDL_CONTROLLER_AXIS_LEFTY, -1.0f)
			.addProp(SDL_CONTROLLER_BUTTON_DPAD_UP, 1.0f)
			.addProp(SDL_CONTROLLER_BUTTON_DPAD_DOWN, -1.0f)
			.addProp(cocos2d::EventKeyboard::KeyCode::KEY_W, 1.0f)
			.addProp(cocos2d::EventKeyboard::KeyCode::KEY_S, -1.0f);

		mInput->addAxis("run", false, "game")
			.addProp(SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 1.0f)
			.addProp(cocos2d::EventKeyboard::KeyCode::KEY_SHIFT, 1.0f);

		mInput->addAction("rope", cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW, "game")
			.addKey(cocos2d::EventKeyboard::KeyCode::KEY_E)
			.addButton(SDL_CONTROLLER_BUTTON_Y);
	}

	mPhysics = std::make_unique<Physics>();
	mPhysicsDebugDrawer = std::make_unique<DebugDrawer>(mLevelScene->mObjectLayer, Physics::PPM);
	mPhysicsDebugDrawer->SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit);
	mPhysics->world->SetDebugDraw(mPhysicsDebugDrawer.get());

	mNetwork = std::make_unique<NetworkManager>();
	mLobby = mNetwork->mLobby.get();
	mNetwork->mOnHandleCreateObject = std::bind(&GameSystem::onHandleCreateNetObject, this, std::placeholders::_1, std::placeholders::_2);
	mNetwork->mOnAllClientsLoadingFinished.bind(this, &GameSystem::onAllClientsLoadingFinished);
	mNetwork->mOnWriteMapInfo.bind(this, &GameSystem::onWriteMapInfo);
	mNetwork->mOnWriteServerInfo.bind(this, &GameSystem::onWriteServerInfo);
	mNetwork->mOnMessage.add(this, &GameSystem::handleMessage);
	mNetwork->mOnErrorMessage.bind(this, &GameSystem::onNetError);
	mNetwork->initConfig(mConfig);
	mNetwork->registerStaticObject(1, nullptr);

	Voting* voting = new Voting(this);
	mLevelScene->addChild(voting->mTextLabel, 20);
	mNetwork->registerService(voting);

	mInput->bindAction("vote", IE_Pressed, std::bind(&GameSystem::onStartVote, this), this);

	mScriptManager = std::make_unique<ScriptManager>(this);


	cocos2d::Size winSize = cocos2d::Director::getInstance()->getWinSize();

	mGameOverLabel = cocos2d::Label::createWithTTF("Game Over", "fonts/Marker Felt.ttf", 80.0f);
	mGameOverLabel->setColor(cocos2d::Color3B::GRAY);
	mGameOverLabel->setPosition(winSize.width / 2, winSize.height / 2);
	mGameOverLabel->setVisible(false);
	mLevelScene->addChild(mGameOverLabel, 20);

	mSpectatingTextLabel = cocos2d::Label::createWithTTF("Spectating:", "fonts/Marker Felt.ttf", 30.0f);
	mSpectatingTextLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_TOP);
	mSpectatingTextLabel->setColor(cocos2d::Color3B::GRAY);
	mSpectatingTextLabel->setPosition(winSize.width / 2.f, winSize.height - 60.f);
	mSpectatingTextLabel->setVisible(false);
	mLevelScene->addChild(mSpectatingTextLabel, 20);
	


	mDebugDrawer = cocos2d::DrawNode::create();
	mLevelScene->mObjectLayer->addChild(mDebugDrawer, 10);

	mDebugTextLabel = cocos2d::Label::createWithTTF("Test", "fonts/Marker Felt.ttf", 20.0f);
	mDebugTextLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_TOP_LEFT);
	mDebugTextLabel->setColor(cocos2d::Color3B::RED);
	mDebugTextLabel->setPosition(10.0f, 350.0f);
	mLevelScene->addChild(mDebugTextLabel, 10);

	mChart = new ChartPanel(mLevelScene);
	mChart->mDrawNode->setAnchorPoint(cocos2d::Vec2::ZERO);
	mChart->mDrawNode->setPosition(cocos2d::Vec2(10, 400));

	setNextMapName("Volley");

	toggleDebug();

	mScoreboard = std::make_unique<Scoreboard>(mLevelScene, this);
}

GameSystem::~GameSystem()
{
}

void GameSystem::setError(const std::string& error)
{
	cocos2d::MessageBox(error.c_str(), "Error");
}

void GameSystem::onNetError(const std::string& error)
{
	cocos2d::MessageBox(error.c_str(), "Network Error!");
}

void GameSystem::addNode(cocos2d::Node* node, int zOrder)
{
	mLevelScene->mObjectLayer->addChild(node, zOrder);
}

Player* GameSystem::spawnPlayer(std::uint16_t clientId, const cocos2d::Vec2& spawnPos)
{
	Parameters spawnParams;
	spawnParams.setInt("clientId", clientId);
	Player* player = static_cast<Player*>(spawnGameObject("player", spawnPos, spawnParams));

	if (mNetwork->isServer())
	{
		auto controller = mNetwork->getPlayerController(clientId);
		if (controller)
		{
			controller->onRespawn();

			GameObject* localPlayer = dynamic_cast<GameObject*>(controller->player);
			if (localPlayer)
			{
				localPlayer->destroyMe();
			}
			controller->player = player;
		}
	}

	return player;
}

void GameSystem::removePlayer(std::uint32_t clientId)
{
	if (!mNetwork->isServer())
		return;

	auto controller = mNetwork->getPlayerController(clientId);
	if (controller)
	{
		GameObject* localPlayer = dynamic_cast<GameObject*>(controller->player);
		if (localPlayer)
		{
			localPlayer->destroyMe();
			controller->player = nullptr;
		}
	}
}

GameObject* GameSystem::spawnGameObject(const std::string& className, const cocos2d::Vec2& pos, Parameters& spawnArgs)
{
	GameObjectFactoryBase* factory = GameObjectDatabase::getFactory(className);
	if (factory)
	{
		if (factory->mCreateOnlyOnServer && mNetwork.get() && !mNetwork->isServer())
		{
			return nullptr;
		}

		GameObject* obj = factory->create(this, pos, spawnArgs);
		if (obj)
		{
			obj->mName = spawnArgs.getString("name");

			if (mNetwork.get() && mNetwork->isServer() && factory->mCreateOnlyOnServer)
			{
				mNetwork->registerObject(obj);
			}
		}

		return obj;
	}

	return nullptr;
}

void GameSystem::addGameObject(GameObject* obj)
{
	if (obj->mLocalIndex != NetworkManager::InvalidNetId)
	{
		return;
	}
	auto it = std::find(mGameObjects.begin(), mGameObjects.end(), obj);
	if (it != mGameObjects.end())
	{
		setError("addGameObject:");
		return;
	}

	obj->mLocalIndex = mGameObjects.size();
	mGameObjects.push_back(obj);
}

void GameSystem::removeGameObject(GameObject* obj)
{
	if (obj->mIsDestroyed)
		return;
	obj->mIsDestroyed = true;
	mRemoveList.push_back(obj);
}

void GameSystem::_removeGameObject(GameObject* obj)
{
	if (!obj)
		return;

	if (obj->mLocalIndex < mGameObjects.size())
	{
		if (obj == mGameObjects[obj->mLocalIndex])
		{
			const std::uint32_t index = obj->mLocalIndex;

			auto it = mGameObjects.begin();
			it = mGameObjects.begin() + index;

			delete (*it);
			(*it) = mGameObjects.back();
			mGameObjects.pop_back();

			it = mGameObjects.begin() + index;
			if (it != mGameObjects.end() && (*it))
			{
				(*it)->mLocalIndex = index;
			}

			return;
		}
	}

	auto it = mGameObjects.begin();
	auto end = mGameObjects.end();
	while (it != end)
	{
		if ((*it) == obj)
		{
			const std::uint32_t index = it - mGameObjects.begin();

			delete (*it);
			(*it) = mGameObjects.back();
			mGameObjects.pop_back();

			it = mGameObjects.begin() + index;
			if (it != mGameObjects.end() && (*it))
			{
				(*it)->mLocalIndex = index;
			}
			
			return;
		}
		++it;
	}
}

void GameSystem::_removeAllGameObjects()
{
	for (size_t i = 0; i < mGameObjects.size(); ++i)
	{
		delete mGameObjects[i];
	}
	mGameObjects.clear();
	mRemoveList.clear();

	setCameraTarget(nullptr);
	mMyPlayer = nullptr;
	mNetwork->mLocalPlayer = nullptr;

	b2Body* body = mPhysics->world->GetBodyList();
	if (body)
	{
		setError("_removeAllGameObjects: body");
	}
	while (body)
	{
		b2Body* next = body->GetNext();

		mPhysics->world->DestroyBody(body);
		body = next;
	}

	bool spriteError = false;
	for (int i = 0; i < mLevelScene->mObjectLayer->getChildrenCount(); ++i)
	{
		cocos2d::Node* node = mLevelScene->mObjectLayer->getChildren().at(i);
		if (dynamic_cast<cocos2d::Sprite*>(node))
		{
			mLevelScene->mObjectLayer->removeChild(node);
			spriteError = true;
		}
	}
	if (spriteError)
	{
		setError("_removeAllGameObjects: sprite");
	}

	if (mNetwork->isServer())
	{
		mNetwork->flushDestroyObjects();
	}
}

GameObject* GameSystem::getGameObject(std::uint32_t index)
{
	if (index >= mGameObjects.size())
		return nullptr;

	return mGameObjects[index];
}

GameObject* GameSystem::getGameObjectByName(const std::string& name)
{
	auto it = mGameObjects.begin();
	auto end = mGameObjects.end();
	while (it != end)
	{
		if ((*it)->getName() == name)
			return *it;
		++it;
	}
	setError("getGameObjectByName: " + name + " not found");
	return nullptr;
}

void GameSystem::setCameraTarget(cocos2d::Node* node)
{
	if (mCameraTarget)
	{
		mLevelScene->mObjectLayer->stopAction(mCameraTarget);
		mCameraTarget = nullptr;
	}

	if (node)
	{
		mCameraTarget = cocos2d::Follow::create(node, cocos2d::Rect(0, 0, mLevelWidth, mLevelHeight));
		mLevelScene->mObjectLayer->runAction(mCameraTarget);
	}
}

void GameSystem::setMyPlayer(Player* player)
{
	mMyPlayer = player;
	mNetwork->mLocalPlayer = player;
}

std::uint32_t GameSystem::calcPlayerNum()
{
	std::uint32_t num = 0;

	auto it = mGameObjects.begin();
	auto end = mGameObjects.end();
	while (it != end)
	{
		if (!(*it)->mIsDestroyed)
		{
			Player* player = dynamic_cast<Player*>((*it));
			if (player && !player->mIsDestroyed)
			{
				num++;
			}
		}
		++it;
	}

	return num;
}

void GameSystem::scoreUser(std::uint16_t clientId)
{
	if (mIsGameOver)
		return;

	Lobby::LobbyUser* user = mLobby->getUserByClientId(clientId);
	if (user)
	{
		user->score++;
	}
}

void GameSystem::scoreTeam(int team)
{
	auto it = mLobby->mLobbyUsers.begin();
	auto end = mLobby->mLobbyUsers.end();
	while (it != end)
	{
		if (it->isConnected)
		{
			NetworkManager::PlayerController* pc = mNetwork->getPlayerController(it->clientId);
			if (pc && pc->team == team)
			{
				it->score++;
			}
		}
		++it;
	}
}

void GameSystem::resetUserScores()
{
	for (size_t i = 0; i < mLobby->mLobbyUsers.size(); ++i)
	{
		mLobby->mLobbyUsers[i].score = 0;
	}
}

void GameSystem::spectating(std::uint32_t clientId)
{
	if (!mNetwork->isServer())
		return;

	NetworkManager::PlayerController* pc = mNetwork->getPlayerController(clientId);
	if (pc)
	{
		pc->spectateStartTime = mNetwork->getTime();

		std::vector<Player*> players;
		findObjects(players);
		if (players.size() > 0)
		{
			GameObject* spectatingObj = nullptr;

			for (size_t i = 0; i < players.size(); ++i)
			{
				spectatingObj = players[i];
				break;
			}

			if (spectatingObj)
			{
				std::uint32_t spectatingId = spectatingObj->mNetId;
				pc->spactatorId = spectatingId;
				pc->spectating = true;

				if (clientId == mNetwork->mClientId)
				{
					spectating(clientId, spectatingObj);
				}
				else
				{
					mNetwork->sendSpectating(clientId, spectatingId);
				}
			}
		}
	}
}

void GameSystem::spectating(std::uint32_t clientId, NetObject* spectator)
{
	if (clientId == mNetwork->mClientId)
	{
		GameObject* spectatorObj = dynamic_cast<GameObject*>(spectator);
		if (spectatorObj)
		{
			setCameraTarget(spectatorObj->mNode);

			mInput->unbindAction("specNext");
			mInput->bindAction("specNext", IE_Pressed, std::bind(&GameSystem::onSpectateNext, this), this);

			Player* player = dynamic_cast<Player*>(spectator);
			if (player)
			{
				std::uint16_t specClientId = player->mClientId;

				Lobby::LobbyUser* lobbyUser = mLobby->getUserByClientId(specClientId);
				if (lobbyUser)
				{
					mSpectatingTextLabel->setString("Spectating: " + lobbyUser->name);
					mSpectatingTextLabel->setVisible(true);
				}
			}
		}
		else
		{
			mSpectatingTextLabel->setVisible(false);
			mInput->unbindAction("specNext");
		}
	}
}

void GameSystem::spectateNext(std::uint32_t clientId)
{
	if (mNetwork->isServer())
	{
		NetworkManager::PlayerController* pc = mNetwork->getPlayerController(clientId);
		if (pc && pc->spectating)
		{
			std::vector<Player*> players;
			findObjects(players);
			if (players.size() > 0)
			{
				GameObject* spectatingObj = nullptr;
				size_t currIdx = 0;
				for (size_t i = 0; i < players.size(); ++i)
				{
					if (pc->spactatorId == players[i]->mNetId)
					{
						currIdx = i;
						break;
					}
				}

				size_t nextIdx = (currIdx + 1) % (players.size());
				spectatingObj = players[nextIdx];
				if (spectatingObj)
				{
					std::uint32_t spectatingId = spectatingObj->mNetId;

					pc->spactatorId = spectatingId;

					if (clientId == mNetwork->mClientId)
					{
						spectating(clientId, spectatingObj);
					}
					else
					{
						mNetwork->sendSpectateNext(clientId, spectatingId);
					}
				}
			}
		}
	}
	else
	{
		mNetwork->sendSpectateNext(clientId, 0);
	}
}

void GameSystem::onSpectateNext()
{
	spectateNext(mNetwork->mClientId);
}

#include "json/filestream.h"
#include "json/filereadstream.h"
#include "json/filewritestream.h"
#include "json/writer.h"

void GameSystem::loadMapNames()
{
	mMapNames.clear();

	FILE* fp = fopen("maps.json", "rb");
	char readBuffer[65536];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

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

	fclose(fp);
}

void GameSystem::setNextMapName(const std::string& mapName)
{
	mNextMapName = mapName;
}

void GameSystem::loadMap(const std::string& mapName)
{
	unloadMap();

	mIsRestarting = false;
	mAllClientsLoadingFinished = false;

	mMapName = mapName;

	if (mNetwork->isServer())
	{
		mNetwork->sendMap(mapName);
	}

	mLevelScene->setPosition(cocos2d::Vec2::ZERO);
	mLevelScene->mObjectLayer->setPosition(cocos2d::Vec2::ZERO);

	FILE* fp = fopen(std::string("maps/" + mapName + ".json").c_str(), "rb");
	rapidjson::FileStream is(fp);

	rapidjson::Document d;
	d.ParseStream(is);

	std::string currMapName = d["level"].GetString();
	std::string modeName = d["mode"].GetString();
	mLevelHeight = d["height"].GetDouble();
	mLevelWidth = d["width"].GetDouble();

	rapidjson::Value& objects = d["objects"];
	if (objects.IsArray())
	{
		for (rapidjson::SizeType i = 0; i < objects.Size(); i++)
		{
			rapidjson::Value& objValue = objects[i];

			Parameters spawnArgs;
			spawnArgs.parse(objValue);

			const std::string type = objValue["type"].GetString();
			cocos2d::Vec2 pos(objValue["px"].GetDouble(), objValue["py"].GetDouble());

			spawnGameObject(type, pos, spawnArgs);
		}
	}

	fclose(fp);

	mGameMode = GameModeDatabase::create(modeName, this);
	mNetwork->setStaticObject(1, mGameMode.get());
	if (!mGameMode)
	{
		setError("Game mode: " + modeName + " not found");
	}

	mIsMapLoaded = true;
}

void GameSystem::unloadMap()
{
	if (!mIsMapLoaded)
		return;
	mIsMapLoaded = false;

	mPrevMapName = mMapName;
	mMapName.clear();

	_removeAllGameObjects();

	mGameMode.reset();
	mNetwork->setStaticObject(1, nullptr);
}

void GameSystem::changeToNextRandomMap()
{
	if (mMapNames.size() == 0) 
	{
		loadMapNames();

		if (mMapNames.size() == 0)
		{
			onGameFinished();
		}
		else
		{
			if (!mMapName.empty())
			{
				auto it = std::find(mMapNames.begin(), mMapNames.end(), mMapName);
				if (it != mMapNames.end())
					mMapNames.erase(it);
			}

			// remove test map
			auto it = std::find(mMapNames.begin(), mMapNames.end(), "test");
			if (it != mMapNames.end())
				mMapNames.erase(it);
		}
	}

	int index = cocos2d::RandomHelper::random_int(0, (int)mMapNames.size() - 1);
	setNextMapName(mMapNames[index]);
	mMapNames.erase(mMapNames.begin() + index);

	wantsRestart();
}

void GameSystem::wantsRestart()
{
	if (mNetwork->isServer())
	{
		mIsRestarting = true;
	}
}

void GameSystem::startGame()
{
	if (cocos2d::Director::getInstance()->getRunningScene() != mLevelScene->mScene)
	{
		cocos2d::Director::getInstance()->pushScene(mLevelScene->mScene);
		mLevelScene->scheduleUpdate();
	}

	mNetwork->resetState();

	Voting* voting = mNetwork->findService<Voting>();
	if (voting)
		voting->reset();

	loadMap(mNextMapName);

	if (mNetwork->isClient())
	{
		mNetwork->sendMapLoadFinished();
	}

	mIsGameRunning = true;
	mIsGameOver = false;
	mGameOverLabel->setVisible(false);

	mGameTick = 0;

	if (mGameMode)
	{
		if (mNetwork->isServer())
		{
			mGameMode->clientConnected(mNetwork->mClientId);
		}
		for (size_t i = 0; i < mNetwork->mConnections.size(); ++i)
		{
			if (mNetwork->mConnections[i] && mNetwork->mConnections[i]->isConnected)
				mGameMode->clientConnected(mNetwork->mConnections[i]->getClientId());
		}

		mGameMode->onStart();

		if (mNetwork->isServer() && mNetwork->getNumClients() == 0)
		{
			onAllClientsLoadingFinished();
		}
	}
}

void GameSystem::exitGame()
{
	if (!mIsGameRunning)
		return;

	mIsGameRunning = false;

	if (mGameMode)
		mGameMode->onEnd();

	unloadMap();

	if (cocos2d::Director::getInstance()->getRunningScene() == mScoreScene)
	{
		mScoreScene = nullptr;

		cocos2d::Director::getInstance()->popScene();
		mLevelScene->unscheduleUpdate();
		cocos2d::Director::getInstance()->popScene();
	}
	else
	{
		if (cocos2d::Director::getInstance()->getRunningScene() == mLevelScene->mScene)
		{
			mLevelScene->unscheduleUpdate();
			cocos2d::Director::getInstance()->popScene();
		}
	}

	if (mNetwork->isServer())
	{
		NetMessage msg(NetMessage::MSG_Lobby_ExitGame);
		mNetwork->send(msg);
	}
}

void GameSystem::onExitGame()
{
	if (!mIsGameRunning)
		return;

	exitGame();

	if (mNetwork->isClient())
	{
		mNetwork->disconnect();
	}
}

void GameSystem::onAllClientsLoadingFinished()
{
	if (mAllClientsLoadingFinished)
		return;
	mAllClientsLoadingFinished = true;

	if (mGameMode)
		mGameMode->onAllClientsLoadingFinished();
}

void GameSystem::readConfig()
{
	FILE* fp = fopen(std::string("config.json").c_str(), "rb");
	if (!fp)
		return;
	char readBuffer[65536];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

	rapidjson::Document d;
	d.ParseStream(is);

	rapidjson::Value& objValue = d;
	mConfig.parse(objValue);

	fclose(fp);
}

void GameSystem::writeConfig()
{
	FILE* fp = fopen(std::string("config.json").c_str(), "wb");
	if (!fp)
		return;
	char writeBuffer[65536];
	rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
	rapidjson::Writer<rapidjson::FileWriteStream> writer(os);

	writer.StartObject();

	auto it = mConfig.mPropMap.begin();
	auto end = mConfig.mPropMap.end();
	while (it != end)
	{
		writer.String(it->first.c_str());

		switch (it->second.type)
		{
		case Parameters::String:
			writer.String(it->second.str.c_str());
			break;
		case Parameters::Bool:
			writer.Bool(mConfig.getBool(it->first));
			break;
		case Parameters::Int:
			writer.Int(mConfig.getInt(it->first));
			break;
		case Parameters::Float:
			writer.Double(mConfig.getFloat(it->first));
			break;
		}

		++it;
	}

	writer.EndObject();

	fclose(fp);
}

void GameSystem::setGameOver()
{
	if (mIsGameOver)
		return;
	mIsGameOver = true;

	mGameOverWaitTime = 0.f;

	mGameOverLabel->setVisible(true);
}

void GameSystem::onGameFinished()
{
	if (mNetwork->isServer())
	{
		mNetwork->sendUserScore();
	}

	mScoreScene = ScoreMenu::createScene(this);
	cocos2d::Director::getInstance()->pushScene(mScoreScene);
}

int gSendDataDebugIndex = GameSystem::addDebugText("Send data (KB/s)", 0.25f, true);
int gRecvDataDebugIndex = GameSystem::addDebugText("Recv data (KB/s)", 0.25f, true);
int gRTTDebugIndex = GameSystem::addDebugText("RTT", 0.25f);
int gPacketsLostDebugIndex = GameSystem::addDebugText("Packets loss", 0.25f);
int gGameTimeDebugIndex = GameSystem::addDebugText("Game time", 0.25f);
int gServerTimeDebugIndex = GameSystem::addDebugText("Server time", 0.25f);
int gDifTimeDebugIndex = GameSystem::addDebugText("diff time", 0.25f);
int gSnapRateDebugIndex = GameSystem::addDebugText("Snap rate", 0.25f);
int gSnapCurrentTimeDebugIndex = GameSystem::addDebugText("Snap current time", 0.1f);
int gSnapTotalTimeDebugIndex = GameSystem::addDebugText("Snap total time", 0.1f);
int gNumSnapsDebugIndex = GameSystem::addDebugText("Num snaps behind", 0.1f);
int gRelevantDistDebugIndex = GameSystem::addDebugText("Relevant dist (F2)", 0.25f);
int gSendFullDebugIndex = GameSystem::addDebugText("SendFullSnapShot (F3)", 0.25f);
int gSyncFrequencyDebugIndex = GameSystem::addDebugText("SyncFrequency (F4, F5)", 0.25f);

#include <fstream>

void GameSystem::update(float deltaTime)
{
	mTime += deltaTime;

	if (mInput->isKeyPressed(cocos2d::EventKeyboard::KeyCode::KEY_TAB))
	{
		mScoreboard->show(true);
	}
	else
	{
		mScoreboard->show(false);
	}

	if (mIsGameOver)
	{
		mGameOverWaitTime += deltaTime;
		if (mGameOverWaitTime >= 2.0f)
		{
			mGameOverLabel->setVisible(false);
			mIsGameOver = false;

			if (mNetwork->isServer())
			{
				mCurrentRoundCounter++;
				if (mCurrentRoundCounter < mTotalRoundCounter)
				{
					changeToNextRandomMap();
				}
				else
				{
					onGameFinished();
				}
			}
		}
	}

	if (mIsGameRunning)
	{
		mGameTick++;
	}

	if (mDebugDrawer)
	{
		mDebugDrawer->clear();
		for (size_t i = 0; i < mDebugLines.size();)
		{
			DebugLine& line = mDebugLines[i];
			if (line.lifeTime > mTime)
			{
				mDebugDrawer->drawLine(line.start, line.end, line.color);
				++i;
			}
			else
			{
				mDebugLines[i] = mDebugLines.back();
				mDebugLines.pop_back();
			}
		}
	}

	if (mDebugTextLabel)
	{
		std::string debugText;
		for (size_t i = 0; i < mDebugTextVec.size(); ++i)
		{
			DebugText& text = mDebugTextVec[i];
			text.updateTime -= deltaTime;
			if (text.updateTime <= 0.0f)
			{
				text.updateTime = text.nextUpdateTime;
				text.drawValue = text.realValue;
			}

			std::stringstream ss;
			ss << text.drawValue;
			debugText += text.text + ": " + ss.str();
			if (text.drawMax)
			{
				std::stringstream ss2;
				ss2 << text.maxValue;
				debugText += " (" + ss2.str() + ")";
			}

			debugText += "\n";
		}
		mDebugTextLabel->setString(debugText);
	}

	mInput->processInput(deltaTime);

	if (mIsGameRunning)
	{
		for (size_t i = 0; i < mGameObjects.size(); ++i)
		{
			if (!mGameObjects[i]->mIsDestroyed)
				mGameObjects[i]->update(deltaTime);
		}
	}

	if (mPhysicsDebugDrawer)
		mPhysicsDebugDrawer->mDrawNode->clear();
	mPhysics->update(deltaTime);

	if (mNetwork)
	{
		if (mInput->isKeyPressed(cocos2d::EventKeyboard::KeyCode::KEY_F2))
		{
			mNetwork->mRelevantDistance += 3.f * deltaTime;
			if (mNetwork->mRelevantDistance >= 50.f)
				mNetwork->mRelevantDistance = 5.f;

			resetDebugText(gSendDataDebugIndex);
			resetDebugText(gRecvDataDebugIndex);
		}

		static bool isKeyF3Down = false;
		if (mInput->isKeyPressed(cocos2d::EventKeyboard::KeyCode::KEY_F3) && !isKeyF3Down)
		{
			isKeyF3Down = true;
			mNetwork->mSendFullSnapShot = !mNetwork->mSendFullSnapShot;

			resetDebugText(gSendDataDebugIndex);
			resetDebugText(gRecvDataDebugIndex);
		}
		else if (!mInput->isKeyPressed(cocos2d::EventKeyboard::KeyCode::KEY_F3))
		{
			isKeyF3Down = false;
		}

		if (mInput->isKeyPressed(cocos2d::EventKeyboard::KeyCode::KEY_F4))
		{
			int freg = 1.f / mNetwork->getSyncFrequency();
			freg--;
			if (freg <= 0)
				freg = 60;
			mNetwork->setSyncFrequency(1.f / (float)freg);

			resetDebugText(gSendDataDebugIndex);
			resetDebugText(gRecvDataDebugIndex);
		}
		if (mInput->isKeyPressed(cocos2d::EventKeyboard::KeyCode::KEY_F5))
		{
			int freg = std::round(1.f / mNetwork->getSyncFrequency());
			freg++;
			if (freg > 60)
				freg = 1;
			mNetwork->setSyncFrequency(1.f / (float)freg);

			resetDebugText(gSendDataDebugIndex);
			resetDebugText(gRecvDataDebugIndex);
		}

		if (mInput->isKeyPressed(cocos2d::EventKeyboard::KeyCode::KEY_F6))
		{
			resetDebugText(gSendDataDebugIndex);
			resetDebugText(gRecvDataDebugIndex);
		}

		mNetwork->mIsGameRunning = mIsGameRunning;
		mNetwork->update(deltaTime);

		static size_t chartIndex = std::numeric_limits<size_t>::max();
		if (chartIndex == std::numeric_limits<size_t>::max())
		{
			chartIndex = mChart->createPoints();
		}
		mChart->addPoint(mNetwork->mDebugLerp, cocos2d::Color4F::RED, chartIndex);

		setDebugText(gSendDataDebugIndex, mNetwork->mTotalSendData);
		setDebugText(gRecvDataDebugIndex, mNetwork->mTotalReceivedData);

		if (mNetwork->getPeer())
		{
			setDebugText(gRTTDebugIndex, mNetwork->getPeer()->roundTripTime);
			setDebugText(gPacketsLostDebugIndex, mNetwork->getPeer()->packetsLost);
			setDebugText(gGameTimeDebugIndex, mNetwork->mInterpolationTime);
			setDebugText(gServerTimeDebugIndex, mNetwork->mSnapServerTime);
			setDebugText(gSnapRateDebugIndex, mNetwork->getSnapRate());
			setDebugText(gSnapCurrentTimeDebugIndex, mNetwork->getSnapCurrentTime());
			setDebugText(gSnapTotalTimeDebugIndex, mNetwork->getSnapshotTotalTime());
			setDebugText(gNumSnapsDebugIndex, mNetwork->calcNumSnapsBehind());
			setDebugText(gDifTimeDebugIndex, mNetwork->mSnapServerTime - mNetwork->mInterpolationTime);
		}
		else if (mNetwork->getHost() && mNetwork->getHost()->peerCount > 0)
		{
			setDebugText(gRTTDebugIndex, mNetwork->getHost()->peers[0].roundTripTime);
			setDebugText(gPacketsLostDebugIndex, mNetwork->getHost()->peers[0].packetsLost);

			NetworkManager::Connection* client = mNetwork->getConnection(mNetwork->getHost()->peers[0].incomingPeerID);
			if (client)
			{
				setDebugText(gGameTimeDebugIndex, client->gameTime);
				setDebugText(gSnapRateDebugIndex, client->snapRate);
				setDebugText(gDifTimeDebugIndex, mNetwork->mSnapServerTime - client->gameTime);
			}
		}

		setDebugText(gRelevantDistDebugIndex, mNetwork->getRelevantDistance());
		setDebugText(gSendFullDebugIndex, mNetwork->mSendFullSnapShot);
		setDebugText(gSyncFrequencyDebugIndex, 1.f / mNetwork->getSyncFrequency());

		static bool isF7Clicked = false;
		static std::uint32_t stateTime = mNetwork->getServiceTime();
		if (mInput->isKeyPressed(cocos2d::EventKeyboard::KeyCode::KEY_F7) && mNetwork->isServer())
		{
			isF7Clicked = true;
		}

		if (isF7Clicked)
		{
			if (mNetwork->getServiceTime() >= stateTime + 1000)
			{
				stateTime = mNetwork->getServiceTime();

				static std::ofstream os;
				static int numWrites = 0;
				if (!os.is_open() && numWrites == 0)
				{
					os.open("stat.txt");
				}
				if (os.is_open() && numWrites < 100)
				{
					numWrites++;
					if (numWrites >= 100)
					{
						os.close();
						isF7Clicked = false;
						numWrites = 0;
					}
					else
					{
						os << (int)(mNetwork->mTotalSendData * 1024.f) << std::endl;
					}
				}
			}
		}
	}

	for (size_t i = 0; i < mRemoveList.size(); ++i)
	{
		_removeGameObject(mRemoveList[i]);
	}
	mRemoveList.clear();

	if (mIsGameRunning && mGameMode)
	{
		mGameMode->update(deltaTime);
	}

	if (mChart)
		mChart->draw();

	if (mIsRestarting)
	{
		startGame();
	}
}

void GameSystem::onStartVote()
{
	Voting* voting = mNetwork->findService<Voting>();
	if (voting)
	{
		voting->startVote();
	}
}

void GameSystem::onWriteMapInfo(NetMessage& msg)
{
	msg.writeString(getCurrMapName());
}

void GameSystem::onWriteServerInfo(NetMessage& msg)
{
	std::uint8_t numClients = mNetwork->getNumClients() + 1;
	bool hasPassword = false;
	std::string mapName = mIsGameRunning ? getCurrMapName() : getNextMap();
	std::string modeName = mGameMode ? mGameMode->getModeName() : "";

	msg.writeString(mNetwork->mServerName);
	msg.writeBool(!mIsGameRunning);
	msg.writeString(mapName);
	msg.writeString(modeName);
	msg.writeUByte(numClients);
	msg.writeUByte(mNetwork->mMaxConnections);
	msg.writeBool(hasPassword);
}

NetObject* GameSystem::onHandleCreateNetObject(std::uint32_t typeId, NetMessage& msg)
{
	cocos2d::Vec2 pos;
	msg.readFloat(pos.x);
	msg.readFloat(pos.y);

	Parameters spawnArgs;
	return GameObjectDatabase::create(typeId, this, pos, spawnArgs);
}

void GameSystem::handleMessage(NetMessage& msg)
{
	const std::uint8_t msgId = msg.mMsgId;
	const std::uint16_t clientId = msg.mClientId;

	if (msgId == NetMessage::MSG_Spectate)
	{
		std::uint32_t objId = NetworkManager::InvalidNetId;

		msg.readUInt(objId);

		NetObject* obj = mNetwork->getObject(objId);
		spectating(mNetwork->mClientId, obj);
	}
	else if (msgId == NetMessage::MSG_SpectateNext)
	{
		std::uint32_t objId = NetworkManager::InvalidNetId;
		msg.readUInt(objId);

		if (mNetwork->isServer())
		{
			spectateNext(clientId);
		}
		else
		{
			NetObject* obj = mNetwork->getObject(objId);
			spectating(mNetwork->mClientId, obj);
		}
	}
	else if (msgId == NetMessage::MSG_UserScore)
	{
		if (mNetwork->isClient())
		{
			onGameFinished();
		}
	}
}

void GameSystem::toggleDebug()
{
	if (mIsDebugEnable)
	{
		mIsDebugEnable = false;

		mPhysicsDebugDrawer->mDrawNode->clear();
		mPhysics->world->SetDebugDraw(nullptr);

		mDebugDrawer->setVisible(false);
		mChart->mDrawNode->setVisible(false);

		mDebugTextLabel->setVisible(false);
		//mDebugTextVec.clear();

		cocos2d::Director::getInstance()->setDisplayStats(false);
	}
	else
	{
		mIsDebugEnable = true;

		mPhysics->world->SetDebugDraw(mPhysicsDebugDrawer.get());

		mDebugDrawer->setVisible(true);
		mChart->mDrawNode->setVisible(true);
		mDebugTextLabel->setVisible(true);

		cocos2d::Director::getInstance()->setDisplayStats(true);
	}
}

void GameSystem::addDebugLine(const cocos2d::Vec2& start, const cocos2d::Vec2& end, const cocos2d::Color4F& color, float lifeTime)
{
	if (mDebugDrawer)
	{
		mDebugLines.push_back(DebugLine());
		DebugLine& line = mDebugLines.back();
		line.lifeTime = mTime + lifeTime;
		line.start = start * Physics::PPM;
		line.end = end * Physics::PPM;
		line.color = color;

		mDebugDrawer->drawLine(start, end, color);
	}
}

void GameSystem::addDebugRect(const cocos2d::Vec2& start, const cocos2d::Vec2& dest, const cocos2d::Color4F& color, float lifeTime)
{
	addDebugLine(cocos2d::Vec2(start.x, start.y), cocos2d::Vec2(dest.x, start.y), color, lifeTime);
	addDebugLine(cocos2d::Vec2(dest.x, start.y), cocos2d::Vec2(dest.x, dest.y), color, lifeTime);
	addDebugLine(cocos2d::Vec2(dest.x, dest.y), cocos2d::Vec2(start.x, dest.y), color, lifeTime);
	addDebugLine(cocos2d::Vec2(start.x, dest.y), cocos2d::Vec2(start.x, start.y), color, lifeTime);
}

int GameSystem::addDebugText(const std::string& text, float updateTime, bool drawMax)
{
	int index = mDebugTextVec.size();
	mDebugTextVec.push_back(DebugText());
	DebugText& debug = mDebugTextVec.back();
	debug.text = text;
	debug.realValue = 0.0;
	debug.maxValue = 0.0;
	debug.drawValue = 0.0;
	debug.updateTime = updateTime;
	debug.nextUpdateTime = updateTime;
	debug.drawMax = drawMax;

	return index;
}

void GameSystem::setDebugText(int index, double value)
{
	if (index >= 0 && index < (int)mDebugTextVec.size())
	{
		mDebugTextVec[index].realValue = value;
		if (value > mDebugTextVec[index].maxValue)
			mDebugTextVec[index].maxValue = value;
	}
}

void GameSystem::resetDebugText(int index)
{
	if (index >= 0 && index < (int)mDebugTextVec.size())
	{
		mDebugTextVec[index].maxValue = 0.0;
	}
}