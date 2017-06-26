#include "Editor.h"

#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"
#include "json/filewritestream.h"
#include "json/filestream.h"

#include "EditorObjects.h"

#include "EditorMenus.h"


Editor::Editor()
	: mDrawNode(0)
	, mFirstSave(true)
	, mDrawDirty(true)
	, mSelectChanged(false)
	, mWantSelect(0)
	, mSelectedSprite(0)
	, mSelectedObject(0)
	, mLeftDown(false)
	, mRightDown(false)
	, mUpDown(false)
	, mDownDown(false)
	, mEditMode(EM_None)
	, mEditAxis(EA_None)
	, mShowGrid(false)
	, mEditLimit(false)
	, mIsCreatingObject(false)
	, mInMenu(false)
{
	mMapName = "MultiMario";
}

Editor::~Editor()
{
	removeAllObjects();

	delete mLimitObject;

	bool spriteError = false;
	for (int i = 0; i < mLevelLayer->getChildrenCount(); ++i)
	{
		cocos2d::Node* node = mLevelLayer->getChildren().at(i);
		if (dynamic_cast<cocos2d::Sprite*>(node))
		{
			mLevelLayer->removeChild(node);
			spriteError = true;
		}
	}
	if (spriteError)
	{
		int a = 0;
	}
}

cocos2d::Scene* Editor::createScene()
{
	// 'scene' is an autorelease object
	auto scene = cocos2d::Scene::create();

	// 'layer' is an autorelease object
	auto layer = Editor::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

bool Editor::init()
{
	if (!Layer::init())
	{
		return false;
	}

	auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
	auto origin = cocos2d::Director::getInstance()->getVisibleOrigin();
	auto winSize = cocos2d::Director::getInstance()->getWinSize();

	mLevelLayer = cocos2d::Layer::create();

	mLevelHeight = winSize.height;
	mLevelWidth = winSize.width * 2 - 100;

	auto backgroundLabel = cocos2d::LayerColor::create(cocos2d::Color4B(128, 128, 128, 180));
	backgroundLabel->setContentSize(cocos2d::Size(420.0f, 28.0f));
	backgroundLabel->setPosition(0.0f, winSize.height - 28.0f);
	addChild(backgroundLabel, 20);

	float offset = 5.0f;
	auto item1 = cocos2d::MenuItemFont::create("load", CC_CALLBACK_1(Editor::onLoad, this));
	item1->setFontSizeObj(20);
	item1->setPosition((item1->getContentSize().width / 2) + offset, winSize.height - (item1->getContentSize().height / 2));
	offset += item1->getContentSize().width + 5.f;

	auto item2 = cocos2d::MenuItemFont::create("save", CC_CALLBACK_1(Editor::onSave, this));
	item2->setFontSizeObj(20);
	item2->setPosition((item2->getContentSize().width / 2) + offset, winSize.height - (item2->getContentSize().height / 2));
	offset += item2->getContentSize().width + 5.f;

	auto item3 = cocos2d::MenuItemFont::create("create", CC_CALLBACK_1(Editor::onCreate, this));
	item3->setFontSizeObj(20);
	item3->setPosition((item3->getContentSize().width / 2) + offset, winSize.height - (item3->getContentSize().height / 2));
	offset += item3->getContentSize().width + 5.f;

	auto item4 = cocos2d::MenuItemFont::create("Limit", CC_CALLBACK_1(Editor::onEditLimit, this));
	item4->setFontSizeObj(20);
	item4->setPosition((item4->getContentSize().width / 2) + offset, winSize.height - (item4->getContentSize().height / 2));
	offset += item4->getContentSize().width + 5.f;

	auto item6 = cocos2d::MenuItemFont::create("Grid", CC_CALLBACK_1(Editor::onToggleGrid, this));
	item6->setFontSizeObj(20);
	item6->setPosition((item6->getContentSize().width / 2) + offset, winSize.height - (item6->getContentSize().height / 2));
	offset += item6->getContentSize().width + 5.f;

	auto item7 = cocos2d::MenuItemFont::create("Unlock", CC_CALLBACK_1(Editor::onUnlockAll, this));
	item7->setFontSizeObj(20);
	item7->setPosition((item7->getContentSize().width / 2) + offset, winSize.height - (item7->getContentSize().height / 2));
	offset += item7->getContentSize().width + 5.f;

	auto item8 = cocos2d::MenuItemFont::create("Mode", CC_CALLBACK_1(Editor::onMode, this));
	item8->setFontSizeObj(20);
	item8->setPosition((item3->getContentSize().width / 2) + offset, winSize.height - (item3->getContentSize().height / 2));
	offset += item8->getContentSize().width + 5.f;

	auto item9 = cocos2d::MenuItemFont::create("Quit", CC_CALLBACK_1(Editor::onQuit, this));
	item9->setFontSizeObj(20);
	item9->setPosition((item8->getContentSize().width / 2) + offset, winSize.height - (item8->getContentSize().height / 2));
	offset += item9->getContentSize().width + 5.f;

	auto menu = cocos2d::Menu::create(item1, item2, item3, item4, item6, item7, item8, item9, nullptr);
	menu->setPosition(origin);
	addChild(menu, 21);

	mDrawNode = cocos2d::DrawNode::create();
	mLevelLayer->addChild(mDrawNode, 9);

	addChild(mLevelLayer);

	mCrayonRenderer = std::make_unique<CrayonRenderer>(mLevelLayer);

	// Select Label
	{
		mSelectLabel = cocos2d::ui::Layout::create();
		mSelectLabel->setLayoutType(cocos2d::ui::LayoutType::VERTICAL);
		mSelectLabel->setBackGroundColorType(cocos2d::ui::Layout::BackGroundColorType::SOLID);
		mSelectLabel->setBackGroundColor(cocos2d::Color3B(128, 128, 128));
		mSelectLabel->setBackGroundColorOpacity(180);
		mSelectLabel->setContentSize(cocos2d::Size(300.0f, 300.0f));
		mSelectLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_TOP_RIGHT);

		mSelectTypeLabel = cocos2d::ui::Text::create("Type:", "fonts/arial.ttf", 20);
		mSelectTypeLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
		mSelectLabel->addChild(mSelectTypeLabel);

		auto nameLayout = cocos2d::ui::Layout::create();
		nameLayout->setLayoutType(cocos2d::ui::LayoutType::HORIZONTAL);
		nameLayout->setContentSize(cocos2d::Size(300.0f, 22.0f));
		mSelectLabel->addChild(nameLayout);

		mSelectNameLabel = cocos2d::ui::Text::create("Name:", "fonts/arial.ttf", 20);
		mSelectNameLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
		nameLayout->addChild(mSelectNameLabel);

		mSelectNameTextField = cocos2d::ui::TextField::create("Insert Name", "fonts/arial.ttf", 20);
		mSelectNameTextField->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
		nameLayout->addChild(mSelectNameTextField);
		mSelectNameTextField->addEventListener([=](Ref* sender, cocos2d::ui::TextField::EventType type)
		{
			switch (type)
			{
			case cocos2d::ui::TextField::EventType::ATTACH_WITH_IME:
				mEditMode = EM_Text;
				break;
			case cocos2d::ui::TextField::EventType::DETACH_WITH_IME:
				mEditMode = EM_None;
				break;
			}
		});

		mPosLabel = cocos2d::ui::Text::create("Pos(G):", "fonts/arial.ttf", 20);
		mPosLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
		mSelectLabel->addChild(mPosLabel);

		mScaleLabel = cocos2d::ui::Text::create("Scale(S):", "fonts/arial.ttf", 20);
		mScaleLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
		mSelectLabel->addChild(mScaleLabel);

		mAnchorLabel = cocos2d::ui::Text::create("Anchor(A):", "fonts/arial.ttf", 20);
		mAnchorLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
		mSelectLabel->addChild(mAnchorLabel);

		auto clampButton = cocos2d::ui::Button::create();
		clampButton->setTitleText("Clamp(C)");
		clampButton->setTitleFontSize(20);
		clampButton->addClickEventListener(CC_CALLBACK_1(Editor::onClampSelect, this));
		mSelectLabel->addChild(clampButton);

		auto lockButton = cocos2d::ui::Button::create();
		lockButton->setTitleText("Lock(L)");
		lockButton->setTitleFontSize(20);
		lockButton->addClickEventListener(CC_CALLBACK_1(Editor::onLockSelect, this));
		mSelectLabel->addChild(lockButton);

		auto deleteButton = cocos2d::ui::Button::create();
		deleteButton->setTitleText("Delete(Del)");
		deleteButton->setTitleFontSize(20);
		deleteButton->addClickEventListener(CC_CALLBACK_1(Editor::onDeleteSelect, this));
		mSelectLabel->addChild(deleteButton);

		auto duplicateButton = cocos2d::ui::Button::create();
		duplicateButton->setTitleText("Duplicate(D)");
		duplicateButton->setTitleFontSize(20);
		duplicateButton->addClickEventListener(CC_CALLBACK_1(Editor::onDuplicateSelect, this));
		mSelectLabel->addChild(duplicateButton);

		auto descLabel = cocos2d::ui::Text::create("Press X or Y to Edit Axis", "fonts/arial.ttf", 20);
		mSelectLabel->addChild(descLabel);

		mSelectLabel->setPosition(cocos2d::Vec2(winSize.width, winSize.height));
		mSelectLabel->setVisible(false);
		addChild(mSelectLabel);
	}

	mLimitObject = new LimitObject(this, "Limit");

	auto keyboardListener = cocos2d::EventListenerKeyboard::create();
	keyboardListener->onKeyPressed = CC_CALLBACK_2(Editor::onKeyPressed, this);
	keyboardListener->onKeyReleased = CC_CALLBACK_2(Editor::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);

	auto listener1 = cocos2d::EventListenerTouchOneByOne::create();
	listener1->setSwallowTouches(true);
	listener1->onTouchBegan = CC_CALLBACK_2(Editor::onTouchBeginTest, this);
	listener1->onTouchMoved = CC_CALLBACK_2(Editor::onTouchMovedTest, this);
	listener1->onTouchEnded = CC_CALLBACK_2(Editor::onTouchEndedTest, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener1, this);

	scheduleUpdate();

	return true;
}

EditObject* Editor::createObject(const std::string& className, const cocos2d::Vec2& pos, int zOrder, const std::string& fileName)
{
	EditObject* obj = 0;
	if (className == "sprite")
	{
		obj = SpriteEditObject::create(this, fileName);
	}
	else
	{
		obj = EditObjectDatabase::create(className, this);
	}

	if (obj)
	{
		addObject(obj, zOrder);
		obj->setPosition(pos);

		onSelect(obj->mNode);
	}
	return obj;
}

void Editor::addObject(EditObject* obj, int zOrder)
{
	mLevelLayer->addChild(obj->mNode, zOrder);
	mObjects.push_back(obj);

	auto listener1 = cocos2d::EventListenerTouchOneByOne::create();
	listener1->setSwallowTouches(true);
	listener1->onTouchBegan = CC_CALLBACK_2(Editor::onTouchBeginTest, this);
	listener1->onTouchMoved = CC_CALLBACK_2(Editor::onTouchMovedTest, this);
	listener1->onTouchEnded = CC_CALLBACK_2(Editor::onTouchEndedTest, this);
	mLevelLayer->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener1, obj->mNode);
	obj->mEventListener = listener1;

	markDrawDirty();
}

void Editor::removeAllObjects()
{
	onSelect(0);
	auto it = mObjects.begin();
	while (it != mObjects.end())
	{
		delete (*it);
		++it;
	}
	mObjects.clear();
}

std::uint32_t Editor::getNextObjectId()
{
	return mCurrObjectId++;
}

EditObject* Editor::findObject(cocos2d::Node* node)
{
	auto it = mObjects.begin();
	auto end = mObjects.end();
	while (it != end)
	{
		if ((*it)->mNode == node)
		{
			return (*it);
		}
		++it;
	}

	return 0;
}

void Editor::resetLevel()
{
	onSelect(0);
	auto it = mObjects.begin();
	while (it != mObjects.end())
	{
		delete (*it);
		++it;
	}
	mObjects.clear();
	mObjectNames.clear();
	mCurrObjectId = 0;
}

void Editor::onSelect(cocos2d::Node* node)
{
	mSelectChanged = false;
	mWantSelect = 0;

	if (node != mSelectedSprite)
	{
		mEditMode = EM_None;
		mEditAxis = EA_None;
	}

	mSelectedSprite = node;

	if (mSelectedSprite)
	{
		mSelectedObject = findObject(node);
	}
	else
	{
		mSelectedObject = 0;
	}

	bool selected = node != 0;
	mSelectLabel->setVisible(selected);

	markDrawDirty();
}

void Editor::onClampSelect(cocos2d::Ref* sender)
{
	if (mSelectedObject)
	{
		clampObject(mSelectedObject);
	}
}

void Editor::onLockSelect(cocos2d::Ref* sender)
{
	if (mSelectedObject)
	{
		if (!mSelectedObject->mIsLocked)
		{
			if (mSelectedObject->mNode)
			{
				mSelectedObject->mNode->setOpacity(150);
				mSelectedObject->mIsLocked = true;
				mSelectedObject->mEventListener->setEnabled(false);
				onSelect(0);
			}
		}
	}
}

void Editor::onDeleteSelect(cocos2d::Ref* sender)
{
	if (mSelectedObject)
	{
		auto it = std::find(mObjects.begin(), mObjects.end(), mSelectedObject);
		if (it != mObjects.end())
		{
			onSelect(0);
			delete (*it);
			mObjects.erase(it);
			return;
		}
	}
}

void Editor::onDuplicateSelect(cocos2d::Ref* sender)
{
	if (mSelectedObject)
	{
		mIsCreatingObject = true;
		mCreateObjectType = mSelectedObject->getTypeName();
		mCreateObjectZOrder = 0;
		if (mSelectedObject->mNode)
			mCreateObjectZOrder = mSelectedObject->mNode->getLocalZOrder();

		SpriteEditObject* sprite = dynamic_cast<SpriteEditObject*>(mSelectedObject);
		if (sprite)
		{
			mCreateObjectFileName = sprite->mFilename;
		}

		mDuplicateObject.enable = true;
		mDuplicateObject.scale = cocos2d::Vec2(mSelectedObject->getScaleX(),
			mSelectedObject->getScaleY());
		mDuplicateObject.anchorPoint = mSelectedObject->getAnchorPoint();

		onSelect(0);
	}
}

void Editor::clampObject(EditObject* obj)
{
	if (!obj)
		return;

	cocos2d::Vec2 pos = obj->getPosition();
	int x = pos.x / 16;
	int y = pos.y / 16;
	float newX = (x * 16.0f) + (16.0f / 2.0f);
	float newY = (y * 16.0f) + (16.0f / 2.0f);
	obj->setPosition(cocos2d::Vec2(newX, newY));

	obj->onPosChange(pos, obj->getPosition());

	markDrawDirty();
}

void Editor::onUnlockAll(cocos2d::Ref* sender)
{
	auto it = mObjects.begin();
	auto end = mObjects.end();
	while (it != end)
	{
		if ((*it)->mIsLocked)
		{
			(*it)->mIsLocked = false;
			(*it)->mEventListener->setEnabled(true);
			(*it)->mNode->setOpacity(255);
		}
		++it;
	}
}

void Editor::loadMapNames()
{
	mMapNames.clear();

	FILE* fp = fopen("maps.json", "rb");
	rapidjson::FileStream is(fp);

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

			//std::string modeName = mapValue["mode"].GetString();
			//mMapModeNames.push_back(modeName);
		}
	}

	fclose(fp);
}

void Editor::saveMapNames()
{
	FILE* fp = fopen("maps.json", "wb");
	if (!fp)
		return;
	char writeBuffer[65536];
	rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
	rapidjson::Writer<rapidjson::FileWriteStream> writer(os);

	writer.StartObject();
	writer.String("maps");

	writer.StartArray();
	{
		for (size_t i = 0; i < mMapNames.size(); ++i)
		{
			writer.StartObject();

			writer.String("name");
			writer.String(mMapNames[i].c_str());

			//writer.String("mode");
			//writer.String(mMapModeNames[i].c_str());

			writer.EndObject();
		}
	}
	writer.EndArray();

	writer.EndObject();

	fclose(fp);
}

void Editor::onLoad(cocos2d::Ref* sender)
{
	loadMapNames();

	cocos2d::Director::getInstance()->pushScene(EditorLoadMapMenu::createScene(this));
}

void Editor::loadMap(const std::string& mapName)
{
	resetLevel();

	FILE* fp = fopen(std::string("maps/" + mapName + ".json").c_str(), "rb");
	rapidjson::FileStream is(fp);

	rapidjson::Document d;
	d.ParseStream(is);

	mMapName = d["level"].GetString();
	mModeName = d["mode"].GetString();

	mLevelHeight = d["height"].GetDouble();
	mLevelWidth = d["width"].GetDouble();

	rapidjson::Value& objects = d["objects"];
	if (objects.IsArray())
	{
		for (rapidjson::SizeType i = 0; i < objects.Size(); i++)
		{
			rapidjson::Value& objValue = objects[i];

			EditObject* obj = 0;

			const std::string type = objValue["type"].GetString();
			cocos2d::Vec2 pos(objValue["px"].GetDouble() * 32.f, objValue["py"].GetDouble() * 32.f);
			cocos2d::Vec2 scale(objValue["sx"].GetDouble(), objValue["sy"].GetDouble());
			int zOrder = objValue["zOrder"].GetInt();

			std::string name;
			if (objValue.HasMember("name"))
				name = objValue["name"].GetString();

			{
				obj = createObject(type, pos, zOrder);
			}

			if (obj)
			{
				obj->setName(name);
				obj->mNode->setPosition(pos);
				if (obj->canScale())
				{
					obj->setScaleX(scale.x);
					obj->setScaleY(scale.y);
				}

				obj->parse(objValue);
			}
		}
	}

	markDrawDirty();

	fclose(fp);

	mFirstSave = false;
	mMapName = mapName;
}

void Editor::onSave(cocos2d::Ref* sender)
{
	if (mFirstSave || mMapName.empty())
	{
		cocos2d::Director::getInstance()->pushScene(EditorSaveMapMenu::createScene(this));
	}
	else
	{
		saveMap(mMapName);
	}
}

void Editor::saveMap(const std::string& mapName)
{
	FILE* fp = fopen(std::string("maps/" + mMapName + ".json").c_str(), "wb");
	if (!fp)
		return;
	char writeBuffer[65536];
	rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
	rapidjson::Writer<rapidjson::FileWriteStream> writer(os);

	writer.StartObject();
	writer.String("level");
	writer.String(mapName.c_str());

	writer.String("mode");
	writer.String(mModeName.c_str());

	writer.String("height");
	writer.Double(mLevelHeight);
	writer.String("width");
	writer.Double(mLevelWidth);

	writer.String("objects");
	writer.StartArray();
	{
		auto it = mObjects.begin();
		auto end = mObjects.end();
		while (it != end)
		{
			cocos2d::Vec2 pos = (*it)->mNode->getPosition();

			writer.StartObject();
			{
				writer.String("type");
				writer.String((*it)->getTypeName().c_str());
				writer.String("name");
				writer.String((*it)->getName().c_str());
				writer.String("px");
				writer.Double(pos.x / 32.f);
				writer.String("py");
				writer.Double(pos.y / 32.f);
				writer.String("sx");
				writer.Double((*it)->getScaleX());
				writer.String("sy");
				writer.Double((*it)->getScaleY());
				writer.String("zOrder");
				writer.Int((*it)->mNode->getLocalZOrder());

				(*it)->serialize(writer);
			}
			writer.EndObject();

			++it;
		}
	}
	writer.EndArray();

	writer.EndObject();

	fclose(fp);
}

void Editor::onCreate(cocos2d::Ref* sender)
{
	cocos2d::Director::getInstance()->pushScene(EditorCreateMenu::createScene(this));
}

void Editor::onEditLimit(cocos2d::Ref* sender)
{
	if (mEditLimit)
	{
		mEditLimit = false;
		mSelectedObject = 0;
		mSelectLabel->setVisible(false);
	}
	else
	{
		mEditLimit = true;
		mSelectedObject = mLimitObject;
		mSelectLabel->setVisible(true);
	}
	markDrawDirty();
}

void Editor::onToggleGrid(cocos2d::Ref* sender)
{
	mShowGrid = mShowGrid ? false : true;

	markDrawDirty();
}

void Editor::onMode(cocos2d::Ref* sender)
{
	cocos2d::Director::getInstance()->pushScene(EditorModeMenu::createScene(this));
}

void Editor::onQuit(cocos2d::Ref* sender)
{
	cocos2d::Director::getInstance()->popScene();
}

bool Editor::onTouchBeginTest(cocos2d::Touch* touch, cocos2d::Event* event)
{
	if (mInMenu)
		return false;

	auto target = static_cast<cocos2d::Sprite*>(event->getCurrentTarget());

	cocos2d::Vec2 locationInNode = target->convertToNodeSpace(touch->getLocation());
	cocos2d::Size s = target->getContentSize();
	cocos2d::Rect rect = cocos2d::Rect(0, 0, s.width, s.height);

	cocos2d::Vec2 levelPosition = touch->getLocation() - mLevelLayer->getPosition();

	if (mEditLimit)
	{
		auto winSize = cocos2d::Director::getInstance()->getWinSize();
		cocos2d::Vec2 pos = touch->getLocation() - mLevelLayer->getPosition();

		mLevelWidth = std::max(pos.x, winSize.width);
		mLevelHeight = std::max(pos.y, winSize.height);

		markDrawDirty();
		return true;
	}

	if (mIsCreatingObject)
	{
		mIsCreatingObject = false;
		EditObject* obj = createObject(mCreateObjectType, levelPosition, mCreateObjectZOrder, mCreateObjectFileName);
		if (obj)
		{
			if (obj->clampOnPlaced())
			{
				clampObject(obj);
			}

			if (mDuplicateObject.enable)
			{
				obj->setScaleX(mDuplicateObject.scale.x);
				obj->setScaleY(mDuplicateObject.scale.y);
				obj->setAnchorPoint(mDuplicateObject.anchorPoint);
			}
		}

		return true;
	}

	if (mSelectedObject)
	{
		if (mSelectedObject->handleTouch(levelPosition))
			return true;
	}

	if (rect.containsPoint(locationInNode))
	{
		target->setOpacity(180);
		cocos2d::Vec2 p = cocos2d::Vec2((0.5f - target->getAnchorPoint().x) * s.width * target->getScaleX(),
			(0.5f - target->getAnchorPoint().y) * s.height * target->getScaleY());
		mSelectStartPos = mLevelLayer->getPosition() + target->getPosition() + p;

		mWantSelect = target;
		mSelectChanged = true;
		return true;
	}

	mSelectChanged = true;
	mWantSelect = 0;

	return false;
}

void Editor::onTouchMovedTest(cocos2d::Touch* touch, cocos2d::Event* event)
{
	if (mEditLimit)
		return;

	auto target = static_cast<cocos2d::Sprite*>(event->getCurrentTarget());
	float scale = 1.0f;

	EditObject* obj = findObject(target);
	if (!obj)
		return;

	if (mEditMode == EM_Position)
	{
		if (mEditAxis == EA_X) obj->setPosition(cocos2d::Vec2(obj->getPosition().x + touch->getDelta().x, obj->getPosition().y));
		else if (mEditAxis == EA_Y) obj->setPosition(cocos2d::Vec2(obj->getPosition().x, obj->getPosition().y + touch->getDelta().y));
		else obj->setPosition(obj->getPosition() + touch->getDelta());
	}
	else if (mEditMode == EM_Scale)
	{
		cocos2d::Size s = target->getContentSize();
		cocos2d::Vec2 objCenterPos = obj->getPosition() +
			cocos2d::Vec2((0.5f - target->getAnchorPoint().x) * s.width * obj->getScaleX(),
			(0.5f - target->getAnchorPoint().y) * s.height * obj->getScaleY());
		cocos2d::Vec2 dir = ((touch->getStartLocation() - mLevelLayer->getPosition()) - objCenterPos);


		scale *= 0.01f;
		if (mEditAxis == EA_X)
		{
			if (dir.x < 0.0f)
				scale *= -1.0f;
			obj->setScaleX(obj->getScaleX() + touch->getDelta().x * scale);
		}
		else if (mEditAxis == EA_Y)
		{
			if (dir.y < 0.0f)
				scale *= -1.0f;
			obj->setScaleY(obj->getScaleY() + touch->getDelta().y * scale);
		}
		else
		{
			float dot = touch->getDelta().dot(dir);
			if (dot < 0.0f)
				scale *= -1.0f;

			obj->setScaleX(obj->getScaleX() + touch->getDelta().length() * scale);
			obj->setScaleY(obj->getScaleY() + touch->getDelta().length() * scale);
		}
	}

	markDrawDirty();
}

void Editor::onTouchEndedTest(cocos2d::Touch* touch, cocos2d::Event* event)
{
	if (mEditLimit)
		return;

	auto target = static_cast<cocos2d::Sprite*>(event->getCurrentTarget());

	if (mSelectedSprite != target)
		return;

	if (mSelectedObject)
	{
		if (mEditMode == EM_Position)
		{
			mSelectedObject->onPosChange(mSelectStartPos, mSelectedObject->getPosition());
		}
	}

	target->setOpacity(255);

	markDrawDirty();
}

void Editor::onKeyPressed(cocos2d::EventKeyboard::KeyCode code, cocos2d::Event* event)
{
	if (mInMenu)
		return;

	if (code == cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW)
		mRightDown = true;
	if (code == cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW)
		mLeftDown = true;
	if (code == cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW)
		mUpDown = true;
	if (code == cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW)
		mDownDown = true;

	if (mSelectedObject)
	{
		if (mEditMode == EM_Text)
		{
			if (code == cocos2d::EventKeyboard::KeyCode::KEY_KP_ENTER || code == cocos2d::EventKeyboard::KeyCode::KEY_ENTER)
			{
				mSelectedObject->setName(mSelectNameTextField->getString());
				mSelectNameTextField->setString(mSelectedObject->getName());
			}
		}
		else
		{
			if (code == cocos2d::EventKeyboard::KeyCode::KEY_G)
			{
				mEditMode = EM_Position;
				mEditAxis = EA_None;
			}
			else if (code == cocos2d::EventKeyboard::KeyCode::KEY_S)
			{
				if (mSelectedObject->canScale())
				{
					mEditMode = EM_Scale;
					mEditAxis = EA_None;
				}
			}
			else if (code == cocos2d::EventKeyboard::KeyCode::KEY_A)
			{
				mEditMode = EM_Anchor;
				mEditAxis = EA_None;
			}
			else if (code == cocos2d::EventKeyboard::KeyCode::KEY_C)
			{
				onClampSelect(0);
			}
			else if (code == cocos2d::EventKeyboard::KeyCode::KEY_L)
			{
				onLockSelect(0);
			}
			else if (code == cocos2d::EventKeyboard::KeyCode::KEY_DELETE)
			{
				onDeleteSelect(0);
			}
			else if (code == cocos2d::EventKeyboard::KeyCode::KEY_D)
			{
				onDuplicateSelect(0);
			}
			if (mEditMode != EM_None)
			{
				//if (mEditAxis == EA_None)
				if (code == cocos2d::EventKeyboard::KeyCode::KEY_X || code == cocos2d::EventKeyboard::KeyCode::KEY_Z)
				{
					if (code == cocos2d::EventKeyboard::KeyCode::KEY_X)
						mEditAxis = EA_X;
					else if (code == cocos2d::EventKeyboard::KeyCode::KEY_Z)
						mEditAxis = EA_Y;
					mEditValue = 0.0f;
					mEditValuePos = 0;
					mEditMinus = false;
				}
				if (mEditAxis != EA_None)
				{
					if (code == cocos2d::EventKeyboard::KeyCode::KEY_KP_ENTER || code == cocos2d::EventKeyboard::KeyCode::KEY_ENTER)
					{
						if (mEditMinus)
							mEditValue *= -1.0f;

						if (mEditMode == EM_Position)
						{
							cocos2d::Vec2 oldPos = mSelectedObject->getPosition();
							cocos2d::Vec2 newPos = mSelectedObject->getPosition();

							mEditValue *= 32.0f;

							if (mEditAxis == EA_X) newPos.x = mEditValue;
							else if (mEditAxis == EA_Y) newPos.y = mEditValue;

							mSelectedObject->setPosition(newPos);
							mSelectedObject->onPosChange(oldPos, newPos);
						}
						else if (mEditMode == EM_Scale)
						{
							if (mEditAxis == EA_X) mSelectedObject->setScaleX(mEditValue);
							else if (mEditAxis == EA_Y) mSelectedObject->setScaleY(mEditValue);
						}
						else if (mEditMode == EM_Anchor)
						{
							if (mEditAxis == EA_X) mSelectedObject->setAnchorPoint(cocos2d::Vec2(mEditValue, mSelectedObject->getAnchorPoint().y));
							else if (mEditAxis == EA_Y) mSelectedObject->setAnchorPoint(cocos2d::Vec2(mSelectedObject->getAnchorPoint().x, mEditValue));
						}
						mEditMode = EM_None;
						mEditAxis = EA_None;
						markDrawDirty();
					}
					else if (code >= cocos2d::EventKeyboard::KeyCode::KEY_0 &&
						code <= cocos2d::EventKeyboard::KeyCode::KEY_9)
					{
						int num = static_cast<int>(code)-static_cast<int>(cocos2d::EventKeyboard::KeyCode::KEY_0);
						if (mEditValuePos == 0)
						{
							if (mEditValue != 0)
								mEditValue *= 10.f;
							mEditValue += num;
						}
						else
						{
							mEditValue += num / (float)(pow(10.0f, mEditValuePos));
							mEditValuePos++;
						}
					}
					else if (code == cocos2d::EventKeyboard::KeyCode::KEY_COMMA ||
						code == cocos2d::EventKeyboard::KeyCode::KEY_PERIOD)
					{
						mEditValuePos = 1;
					}
					else if (code == cocos2d::EventKeyboard::KeyCode::KEY_MINUS ||
						code == cocos2d::EventKeyboard::KeyCode::KEY_KP_MINUS ||
						code == cocos2d::EventKeyboard::KeyCode::KEY_SLASH)
					{
						mEditMinus = mEditMinus ? false : true;
					}
				}
			}
		}
	}

}

void Editor::onKeyReleased(cocos2d::EventKeyboard::KeyCode code, cocos2d::Event* event)
{
	if (code == cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW)
		mRightDown = false;
	if (code == cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW)
		mLeftDown = false;
	if (code == cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW)
		mUpDown = false;
	if (code == cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW)
		mDownDown = false;
}

void Editor::update(float deltaTime)
{
	if (mSelectChanged)
	{
		onSelect(mWantSelect);
	}

	if (mRightDown)
	{
		mLevelLayer->setPosition(mLevelLayer->getPosition() + cocos2d::Vec2(-10, 0));
		markDrawDirty();
	}
	if (mLeftDown)
	{
		mLevelLayer->setPosition(mLevelLayer->getPosition() + cocos2d::Vec2(10, 0));
		markDrawDirty();
	}
	if (mUpDown)
	{
		mLevelLayer->setPosition(mLevelLayer->getPosition() + cocos2d::Vec2(0, -10));
		markDrawDirty();
	}
	if (mDownDown)
	{
		mLevelLayer->setPosition(mLevelLayer->getPosition() + cocos2d::Vec2(0, 10));
		markDrawDirty();
	}

	if (mDrawDirty)
	{
		mDrawDirty = false;
		mDrawNode->clear();

		auto s = cocos2d::Director::getInstance()->getWinSize();

		if (mShowGrid)
		{
			int stepsX = (mLevelWidth / 16) + 1;
			int stepsY = (mLevelHeight / 16) + 1;
			for (int x = 0; x < stepsX; x++)
			{
				mDrawNode->drawLine(cocos2d::Vec2(x * 16.0f, 0), cocos2d::Vec2(x * 16.0f, mLevelHeight), cocos2d::Color4F::BLUE);
			}
			for (int y = 0; y < stepsY; y++)
			{
				mDrawNode->drawLine(cocos2d::Vec2(0, y * 16.0f), cocos2d::Vec2(mLevelWidth, y * 16.0f), cocos2d::Color4F::BLUE);
			}
		}

		auto it = mObjects.begin();
		auto end = mObjects.end();
		while (it != end)
		{
			(*it)->draw();
			++it;
		}


		float x = s.width * 2 - 100;
		float y = s.height;
		cocos2d::Vec2 vertices[] = { cocos2d::Vec2(0, 0), cocos2d::Vec2(mLevelWidth - 0, 0), cocos2d::Vec2(mLevelWidth - 0, mLevelHeight - 0), cocos2d::Vec2(0, mLevelHeight - 0) };
		mDrawNode->drawPoly(vertices, 4, true, cocos2d::Color4F::BLUE);

		if (mSelectedObject)
		{
			mSelectTypeLabel->setString("Type: " + mSelectedObject->getTypeName());
			mSelectNameTextField->setString(mSelectedObject->getName());

			std::stringstream ss;
			ss << mSelectedObject->getPosition().x / 32.f;
			std::string x = ss.str();
			ss.str("");
			ss << mSelectedObject->getPosition().y / 32.f;
			std::string y = ss.str();
			mPosLabel->setString("Pos(G): " + x + " " + y);

			ss.str("");
			ss << mSelectedObject->getScaleX();
			x = ss.str();
			ss.str("");
			ss << mSelectedObject->getScaleY();
			y = ss.str();
			mScaleLabel->setString("Scale(S): " + x + " " + y);

			ss.str("");
			ss << mSelectedObject->getAnchorPoint().x;
			x = ss.str();
			ss.str("");
			ss << mSelectedObject->getAnchorPoint().y;
			y = ss.str();
			mAnchorLabel->setString("Anchor(A): " + x + " " + y);

			cocos2d::Rect rect = mSelectedObject->getBoundingBox();
			mDrawNode->drawRect(rect.origin, rect.origin + rect.size, cocos2d::Color4F::WHITE);
		}
	}
}

void Editor::onEnter()
{
	Layer::onEnter();
}

void Editor::onEnterTransitionDidFinish()
{
	Layer::onEnterTransitionDidFinish();
}

void Editor::onExitTransitionDidStart()
{
	Layer::onExitTransitionDidStart();
}

void Editor::onExit()
{
	Layer::onExit();
}