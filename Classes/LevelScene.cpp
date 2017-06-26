#include "LevelScene.h"

#include "GameSystem.h"
#include "Input.h"

USING_NS_CC;

LevelScene::LevelScene()
{
}

LevelScene::~LevelScene()
{
}

Scene* LevelScene::createScene()
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = LevelScene::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

bool LevelScene::init()
{
	if (!Layer::init())
	{
		return false;
	}

	mScene = Scene::create();
	mScene->addChild(this);

	mObjectLayer = cocos2d::Layer::create();
	addChild(mObjectLayer);

	return true;
}

void LevelScene::registerInput(Input* input)
{
	auto keyboardListener = EventListenerKeyboard::create();
	keyboardListener->onKeyPressed = CC_CALLBACK_2(Input::onKeyPressed, input);
	keyboardListener->onKeyReleased = CC_CALLBACK_2(Input::onKeyReleased, input);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);

	auto mouseListener = EventListenerMouse::create();
	mouseListener->onMouseMove = CC_CALLBACK_1(Input::onMouseMove, input);
	mouseListener->onMouseUp = CC_CALLBACK_1(Input::onMouseUp, input);
	mouseListener->onMouseDown = CC_CALLBACK_1(Input::onMouseDown, input);
	mouseListener->onMouseScroll = CC_CALLBACK_1(Input::onMouseScroll, input);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}

void LevelScene::update(float deltaTime)
{
	mGame->update(deltaTime);
}