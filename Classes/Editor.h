#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "cocos2d.h"
#include "ui\CocosGUI.h"

#include "CrayonRenderer.h"

class EditObject;
class ParallaxObject;

class Editor : public cocos2d::Layer
{
public:
	enum eEditMode
	{
		EM_None,
		EM_Position,
		EM_Scale,
		EM_Anchor,
		EM_Text,
	};

	enum eEditAxis
	{
		EA_None,
		EA_X,
		EA_Y,
	};

	struct ParallaxLayer
	{
		ParallaxLayer()
		{}
		ParallaxLayer(int _index, int _zOrder, const cocos2d::Vec2& _ratio)
			: index(_index)
			, zOrder(_zOrder)
			, ratio(_ratio)
		{}

		int index;
		int zOrder;
		cocos2d::Vec2 ratio;
	};

	Editor();
	virtual ~Editor();

	static cocos2d::Scene* createScene();
	virtual bool init();

	void markDrawDirty() { mDrawDirty = true; }

	EditObject* createObject(const std::string& className, const cocos2d::Vec2& pos, int zOrder, const std::string& fileName = "");
	void addObject(EditObject* obj, int zOrder = 5);

	void removeAllObjects();

	std::uint32_t getNextObjectId();

	EditObject* findObject(cocos2d::Node* node);

	void resetLevel();

	void onSelect(cocos2d::Node* node);
	void onClampSelect(cocos2d::Ref* sender);
	void onLockSelect(cocos2d::Ref* sender);
	void onDeleteSelect(cocos2d::Ref* sender);
	void onDuplicateSelect(cocos2d::Ref* sender);

	void clampObject(EditObject* obj);

	void onUnlockAll(cocos2d::Ref* sender);

	void loadMapNames();
	void saveMapNames();

	void onLoad(cocos2d::Ref* sender);
	void loadMap(const std::string& mapName);

	void onSave(cocos2d::Ref* sender);
	void saveMap(const std::string& mapName);

	void onCreate(cocos2d::Ref* sender);
	void onEditLimit(cocos2d::Ref* sender);
	void onToggleGrid(cocos2d::Ref* sender);

	void onMode(cocos2d::Ref* sender);

	void onQuit(cocos2d::Ref* sender);

	bool onTouchBeginTest(cocos2d::Touch* touch, cocos2d::Event* event);
	void onTouchMovedTest(cocos2d::Touch* touch, cocos2d::Event* event);
	void onTouchEndedTest(cocos2d::Touch* touch, cocos2d::Event* event);

	void onKeyPressed(cocos2d::EventKeyboard::KeyCode code, cocos2d::Event* event);
	void onKeyReleased(cocos2d::EventKeyboard::KeyCode code, cocos2d::Event* event);

	void update(float deltaTime);

	virtual void onEnter() override;
	virtual void onEnterTransitionDidFinish() override;
	virtual void onExitTransitionDidStart() override;
	virtual void onExit() override;

	CREATE_FUNC(Editor);

	cocos2d::Layer* mLevelLayer;

	std::string mMapName;
	std::string mModeName;

	float mLevelHeight;
	float mLevelWidth;
	class LimitObject* mLimitObject;
	bool mEditLimit;

	bool mFirstSave;

	bool mDrawDirty;
	cocos2d::DrawNode* mDrawNode;
	bool mSelectChanged;
	cocos2d::Node* mWantSelect;
	cocos2d::Node* mSelectedSprite;
	EditObject* mSelectedObject;
	cocos2d::Vec2 mSelectStartPos;

	bool mShowGrid;

	eEditMode mEditMode;
	eEditAxis mEditAxis;
	float mEditValue;
	int mEditValuePos;
	bool mEditMinus;

	bool mLeftDown;
	bool mRightDown;
	bool mUpDown;
	bool mDownDown;

	cocos2d::ui::Layout* mSelectLabel;
	cocos2d::ui::Text* mSelectTypeLabel;
	cocos2d::ui::Text* mSelectNameLabel;
	cocos2d::ui::TextField* mSelectNameTextField;
	cocos2d::ui::Text* mPosLabel;
	cocos2d::ui::Text* mScaleLabel;
	cocos2d::ui::Text* mAnchorLabel;

	std::vector<EditObject*> mObjects;
	std::map<std::string, EditObject*> mObjectNames;

	std::vector<std::string> mMapNames;
	std::vector<std::string> mMapModeNames;

	bool mIsCreatingObject;
	std::string mCreateObjectType;
	int mCreateObjectZOrder;
	std::string mCreateObjectFileName;

	struct DuplicateObject
	{
		bool enable = false;
		cocos2d::Vec2 scale;
		cocos2d::Vec2 anchorPoint;
	};
	DuplicateObject mDuplicateObject;

	bool mInMenu;

	std::unique_ptr<CrayonRenderer> mCrayonRenderer;

	std::uint32_t mCurrObjectId = 0;
};

#endif