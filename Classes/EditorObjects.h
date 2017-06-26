#ifndef __EDITOROBJECTS_H__
#define __EDITOROBJECTS_H__

#include "cocos2d.h"
#include "json/document.h"
#include "json/writer.h"
#include "json/filewritestream.h"

class Editor;
class CrayonRenderer;
class DynamicCrayonRenderer;

class EditObject
{
public:
	EditObject(Editor* editor, const std::string& className);
	virtual ~EditObject();

	virtual std::string generateName();

	virtual std::string getTypeName() const { return mTypeName; };
	std::string getName() const { return mName; }
	void setName(const std::string& name);

	virtual bool canScale() { return true; }
	virtual bool canRotate() { return false; }
	virtual bool clampOnPlaced() { return false; }

	virtual bool isCreatingFinished() { return true; }

	virtual cocos2d::Vec2 getPosition();
	virtual void setPosition(const cocos2d::Vec2& pos);

	virtual float getScaleX() const;
	virtual void setScaleX(float scale);
	virtual float getScaleY() const;
	virtual void setScaleY(float scale);

	virtual float getRotation();
	virtual void setRotation(float rot);

	virtual cocos2d::Vec2 getAnchorPoint() const;
	virtual void setAnchorPoint(const cocos2d::Vec2& point);

	virtual cocos2d::Rect getBoundingBox() const;

	virtual void onPosChange(const cocos2d::Vec2& oldPos, const cocos2d::Vec2& newPos) {}
	virtual void onScaleChanged() {};

	virtual bool containsPoint(const cocos2d::Vec2& point) const { return true; }

	virtual bool handleTouch(const cocos2d::Vec2& pos) { return false; }

	virtual void parse(rapidjson::Value& obj) {}
	virtual void serialize(rapidjson::Writer<rapidjson::FileWriteStream>& writer) const {}

	virtual void draw() {};

	Editor* mEditor = nullptr;
	cocos2d::Node* mNode = nullptr;
	cocos2d::EventListener* mEventListener = nullptr;

	bool mIsLocked = false;

	std::string mTypeName;
	std::string mClassName;
	std::string mName;
};


class PlayerObject : public EditObject
{
public:
	PlayerObject(Editor* editor, const std::string& className);

	bool canScale() override { return false; }

	void draw();
};

class SpriteEditObject : public EditObject
{
public:
	SpriteEditObject(Editor* editor, const std::string& fileName);

	static SpriteEditObject* create(Editor* editor, const std::string& fileName = "")
	{
		SpriteEditObject* obj = new SpriteEditObject(editor, fileName);
		return obj;
	}

	void parse(rapidjson::Value& obj);
	void serialize(rapidjson::Writer<rapidjson::FileWriteStream>& writer) const;

	std::string mFilename;
};

class BoxColliderObject : public EditObject
{
public:
	BoxColliderObject(Editor* editor, const std::string& className);

	void draw();
};

class PolygonColliderObject : public EditObject
{
public:
	PolygonColliderObject(Editor* editor, const std::string& className);

	bool handleTouch(const cocos2d::Vec2& pos);

	virtual bool isCreatingFinished();

	void draw();

	void computeCentroid();

	void parse(rapidjson::Value& obj);
	void serialize(rapidjson::Writer<rapidjson::FileWriteStream>& writer) const;

	std::vector<cocos2d::Vec2> mPoints;

	bool mCreateFinished = false;
	bool mIsPolygon = false;

	std::unique_ptr<CrayonRenderer> mCrayonRenderer;
};

class KillZoneObject : public EditObject
{
public:
	KillZoneObject(Editor* editor, const std::string& className);

	void draw();
};


class CoinObject : public EditObject
{
public:
	CoinObject(Editor* editor, const std::string& className);

	bool canScale() override { return false; }
	virtual bool clampOnPlaced() { return true; }
};


class MovingPlatformEditObject : public EditObject
{
public:
	MovingPlatformEditObject(Editor* editor, const std::string& className);

	bool canScale() override { return false; }

	virtual cocos2d::Vec2 getAnchorPoint() const;
	virtual void setAnchorPoint(const cocos2d::Vec2& point);

	void parse(rapidjson::Value& obj);
	void serialize(rapidjson::Writer<rapidjson::FileWriteStream>& writer) const;

	void draw();

	cocos2d::Vec2 mEndPos;

	std::unique_ptr<DynamicCrayonRenderer> mCrayonRenderer;
};

class LimitObject : public EditObject
{
public:
	LimitObject(Editor* editor, const std::string& className);

	bool canScale() override { return false; }

	virtual cocos2d::Vec2 getPosition() override;
	virtual void setPosition(const cocos2d::Vec2& pos) override;
};

class EditObjectFactoryBase
{
public:
	EditObjectFactoryBase(const std::string& name, const std::string& type, int zOrder);

	virtual EditObject* create(Editor* editor) = 0;

	std::string mName;
	std::string mType;
	int mZOrder;
};

template <class T>
class EditObjectFactory : public EditObjectFactoryBase
{
public:
	EditObjectFactory(const std::string& name, const std::string& type, int zOrder)
		: EditObjectFactoryBase(name, type, zOrder)
	{}

	EditObject* create(Editor* editor) { return new T(editor, mName); }
};


class EditObjectDatabase
{
public:
	static void registerObject(const std::string& type, EditObjectFactoryBase* factory);

	static EditObject* create(const std::string& type, Editor* editor);
	static EditObjectFactoryBase* getFactory(const std::string& type);

	static EditObjectDatabase* getInstance()
	{
		static EditObjectDatabase* instance = 0;
		if (!instance)
		{
			instance = new EditObjectDatabase();
		}
		return instance;
	}

	std::map<std::string, EditObjectFactoryBase*> mFactorys;
};

#endif