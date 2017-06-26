#ifndef __EDITORMENUS_H__
#define __EDITORMENUS_H__

#include "cocos2d.h"
#include "ui\CocosGUI.h"

class Editor;

class EditorCreateMenu : public cocos2d::Layer
{
public:
	struct ObjectCreator
	{
		ObjectCreator()
		{}

		ObjectCreator(cocos2d::MenuItemFont* _item, const std::string& _name, const std::string& _type, int _zOrder)
			: name(_name)
			, type(_type)
			, zOrder(_zOrder)
			, item(_item)
		{}

		cocos2d::MenuItemFont* item;
		std::string name;
		std::string type;
		int zOrder;
	};

public:
	EditorCreateMenu();
	~EditorCreateMenu();

	static cocos2d::Scene* createScene(Editor* editor);

	virtual bool init();

	virtual void onEnter() override;
	virtual void onExit() override;

	void registerObject(const std::string& name, const std::string& type, int zOrder);

	void onCreateObject(cocos2d::Ref* sender);
	void onBack(cocos2d::Ref* sender);

	CREATE_FUNC(EditorCreateMenu);

	cocos2d::Scene* mScene = nullptr;
	Editor* mEditor = nullptr;

	cocos2d::MenuItemFont* mSpriteItem;

	std::vector<ObjectCreator> mObjectCreaters;
};

class EditorSpriteCreateMenu : public cocos2d::Layer
{
public:
	static cocos2d::Scene* createScene(Editor* editor);

	virtual bool init();

	virtual void onEnter() override;
	virtual void onExit() override;

	void onSetZOrder(cocos2d::Ref* sender);
	void onCreateSprite(cocos2d::Ref* sender);
	void onBack(cocos2d::Ref* sender);

	CREATE_FUNC(EditorSpriteCreateMenu);

	cocos2d::Scene* mScene = nullptr;
	Editor* mEditor = nullptr;

	cocos2d::ui::TextField* mTextField = nullptr;
	int mCurrZOrder = -1;
	cocos2d::Vector<cocos2d::MenuItem*> mZOrderMenus;
};

class EditorLoadMapMenu : public cocos2d::Layer
{
public:
	static cocos2d::Scene* createScene(Editor* editor);

	virtual bool init();
	void initMenus();

	virtual void onEnter() override;
	virtual void onExit() override;

	void onLoad(cocos2d::Ref* sender);
	void onBack(cocos2d::Ref* sender);

	CREATE_FUNC(EditorLoadMapMenu);

	cocos2d::Scene* mScene = nullptr;
	Editor* mEditor = nullptr;

	std::vector<cocos2d::MenuItem*> mMapNamesMenus;
};

class EditorSaveMapMenu : public cocos2d::Layer
{
public:
	static cocos2d::Scene* createScene(Editor* editor);

	virtual bool init();

	virtual void onEnter() override;
	virtual void onExit() override;

	void onSave(cocos2d::Ref* sender);
	void onBack(cocos2d::Ref* sender);

	CREATE_FUNC(EditorSaveMapMenu);

	cocos2d::Scene* mScene = nullptr;
	Editor* mEditor = nullptr;

	cocos2d::ui::TextField* mTextField = nullptr;
};

class EditorModeMenu : public cocos2d::Layer
{
public:
	struct Mode
	{
		Mode(cocos2d::MenuItemFont* _item, const std::string& _name)
			: item(_item)
			, name(_name)
		{}

		cocos2d::MenuItemFont* item;
		std::string name;
	};

public:
	EditorModeMenu();
	~EditorModeMenu();

	static cocos2d::Scene* createScene(Editor* editor);

	virtual bool init();

	virtual void onEnter() override;
	virtual void onExit() override;

	void registerMode(const std::string& name);

	void onSetMode(cocos2d::Ref* sender);
	void onBack(cocos2d::Ref* sender);

	CREATE_FUNC(EditorModeMenu);

	cocos2d::Scene* mScene = nullptr;
	Editor* mEditor = nullptr;

	std::vector<Mode> mModes;
};

#endif