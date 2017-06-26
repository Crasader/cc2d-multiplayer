#include "MenuBase.h"

#include "GameSystem.h"
#include "Input.h"
#include "ui\CocosGUI.h"

MenuBase::MenuBase()
{

}

MenuBase::~MenuBase()
{
	if (mGame)
		mGame->mInput->unbindAll(this);
}

void MenuBase::init(GameSystem* game)
{
	mGame = game;
}

void MenuBase::setEnableInput(bool enable)
{
	if (mInputEnable == enable)
		return;

	mInputEnable = enable;

	if (mGame && mGame->mInput)
	{
		if (enable)
		{
			mGame->mInput->bindAction("menuEnter", IE_Pressed, std::bind(&MenuBase::onMenuEnter, this), this);
			mGame->mInput->bindAction("menuBack", IE_Pressed, std::bind(&MenuBase::onMenuBack, this), this);
			mGame->mInput->bindAction("menuUp", IE_Pressed, std::bind(&MenuBase::onMenuUp, this), this);
			mGame->mInput->bindAction("menuDown", IE_Pressed, std::bind(&MenuBase::onMenuDown, this), this);

			scheduleUpdate();

			mEventListener = cocos2d::EventListenerKeyboard::create();
			mEventListener->onKeyPressed = CC_CALLBACK_2(Input::onKeyPressed, mGame->mInput.get());
			mEventListener->onKeyReleased = CC_CALLBACK_2(Input::onKeyReleased, mGame->mInput.get());
			_eventDispatcher->addEventListenerWithSceneGraphPriority(mEventListener, this);
		}
		else
		{
			unscheduleUpdate();
			if (mGame && mGame->mInput)
			{
				mGame->mInput->unbindAll(this);

				_eventDispatcher->removeEventListener(mEventListener);
			}
		}
	}
}

void MenuBase::addControlButton(cocos2d::MenuItem* menu)
{
	mMenus.pushBack(menu);
}

void MenuBase::onMenuEnter()
{
	if (cocos2d::Director::getInstance()->getRunningScene() != getScene())
		return;

	int selectedMenuIndex = -1;
	for (int i = 0; i < mMenus.size(); ++i)
	{
		if (mMenus.at(i)->isSelected())
		{
			selectedMenuIndex = i;
		}
	}

	if (selectedMenuIndex != -1)
	{
		mMenus.at(selectedMenuIndex)->activate();
	}
}

void MenuBase::onMenuBack()
{
	if (cocos2d::Director::getInstance()->getRunningScene() != getScene())
		return;

	cocos2d::Director::getInstance()->popScene();
}

void MenuBase::onMenuUp()
{
	if (cocos2d::Director::getInstance()->getRunningScene() != getScene())
		return;

	if (mMenus.size() <= 1)
		return;

	int selectedMenuIndex = 0;
	for (int i = 0; i < mMenus.size(); ++i)
	{
		if (mMenus.at(i)->isSelected() && mMenus.at(i)->isEnabled())
		{
			mMenus.at(i)->unselected();
			selectedMenuIndex = i;
		}
	}

	bool isValid = false;
	int iteration = 0;
	while (iteration != mMenus.size())
	{
		selectedMenuIndex--;
		if (selectedMenuIndex < 0)
			selectedMenuIndex = mMenus.size() - 1;

		if (mMenus.at(selectedMenuIndex)->isEnabled())
		{
			isValid = true;
			break;
		}
	}

	if (isValid)
	{
		mMenus.at(selectedMenuIndex)->selected();
	}
}

void MenuBase::onMenuDown()
{
	if (cocos2d::Director::getInstance()->getRunningScene() != getScene())
		return;

	if (mMenus.size() <= 1)
		return;

	int selectedMenuIndex = 0;
	for (int i = 0; i < mMenus.size(); ++i)
	{
		if (mMenus.at(i)->isSelected() && mMenus.at(i)->isEnabled())
		{
			mMenus.at(i)->unselected();
			selectedMenuIndex = i;
		}
	}

	bool isValid = false;
	int iteration = 0;
	while (iteration != mMenus.size())
	{
		selectedMenuIndex++;
		if (selectedMenuIndex >= mMenus.size())
			selectedMenuIndex = 0;

		if (mMenus.at(selectedMenuIndex)->isEnabled())
		{
			isValid = true;
			break;
		}
	}

	if (isValid)
	{
		mMenus.at(selectedMenuIndex)->selected();
	}
}

void MenuBase::update(float deltaTime)
{
	if (cocos2d::Director::getInstance()->getRunningScene() != getScene())
		return;

	if (mGame && mGame->mInput)
	{
		mGame->mInput->processInput(deltaTime);

		if (mUpdateMenuTimer > 0.0f)
		{
			mUpdateMenuTimer -= deltaTime;
		}
		else
		{
			float axisValue = mGame->mInput->getAxisValue("menuUp");
			if (axisValue > 0.2f || axisValue < -0.2f)
			{
				mUpdateMenuTimer = 0.2f;

				if (axisValue > 0.0f)
					onMenuUp();
				else
					onMenuDown();
			}
		}
	}
}

void MenuBase::onEnter()
{
	mGame->mInput->clearPendingActions();

	Layer::onEnter();

	setEnableInput(true);
}

void MenuBase::onExit()
{
	Layer::onExit();

	setEnableInput(false);
}