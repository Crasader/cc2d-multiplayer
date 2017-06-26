#include "EditorObjects.h"

#include "Editor.h"
#include "Physics.h"

#include "DynamicCrayonRenderer.h"

EditObject::EditObject(Editor* editor, const std::string& className)
	: mNode(0)
	, mEditor(editor)
	, mClassName(className)
	, mEventListener(0)
	, mIsLocked(false)
{
}

EditObject::~EditObject()
{
	if (mNode)
	{
		mEditor->mLevelLayer->removeChild(mNode);
	}
}

std::string EditObject::generateName()
{
	std::stringstream ss;
	ss << mEditor->getNextObjectId();
	return mClassName + ss.str();
}

void EditObject::setName(const std::string& name)
{
	if (mName == name)
		return;

	if (!mName.empty())
	{
		auto it = mEditor->mObjectNames.find(mName);
		if (it != mEditor->mObjectNames.end())
		{
			mEditor->mObjectNames.erase(it);
		}
	}

	if (!name.empty())
	{
		std::string newName = name;

		while (true)
		{
			auto it = mEditor->mObjectNames.find(newName);
			if (it == mEditor->mObjectNames.end())
				break;

			std::stringstream ss;
			ss << mEditor->getNextObjectId();
			newName = name + ss.str();
		}

		mName = newName;
		mEditor->mObjectNames.insert(std::make_pair(newName, this));
	}
}

cocos2d::Vec2 EditObject::getPosition()
{
	if (mNode)
		return mNode->getPosition();
	return cocos2d::Vec2::ZERO;
}

void EditObject::setPosition(const cocos2d::Vec2& pos)
{
	if (mNode)
		mNode->setPosition(pos);
}

float EditObject::getScaleX() const
{
	if (mNode)
		return mNode->getScaleX();
	return 0.f;
}

void EditObject::setScaleX(float scale)
{
	if (mNode)
		mNode->setScaleX(scale);
}

float EditObject::getScaleY() const
{
	if (mNode)
		return mNode->getScaleY();
	return 0.f;
}

void EditObject::setScaleY(float scale)
{
	if (mNode)
		mNode->setScaleY(scale);
}

float EditObject::getRotation()
{
	if (mNode)
		return mNode->getRotation();
	return 0;
}

void EditObject::setRotation(float rot)
{
	if (mNode)
		mNode->setRotation(rot);
}

cocos2d::Vec2 EditObject::getAnchorPoint() const
{
	if (mNode)
		return mNode->getAnchorPoint();
	return cocos2d::Vec2::ZERO;
}

void EditObject::setAnchorPoint(const cocos2d::Vec2& point)
{
	if (mNode)
		return mNode->setAnchorPoint(point);
}

cocos2d::Rect EditObject::getBoundingBox() const
{
	if (mNode)
		return mNode->getBoundingBox();
	return cocos2d::Rect::ZERO;
}


PlayerObject::PlayerObject(Editor* editor, const std::string& className)
	: EditObject(editor, className)
{
	auto sprite = cocos2d::Sprite::create("CharStand.png");
	mNode = sprite;
	mNode->setScale(0.75f);
}

void PlayerObject::draw()
{
	cocos2d::Vec2 pos = mNode->getPosition();
	mEditor->mDrawNode->drawRect(pos - cocos2d::Vec2(0.5f * Physics::PPM, 1.5f * Physics::PPM),
		pos + cocos2d::Vec2(0.5f * Physics::PPM, 1.5f * Physics::PPM),
		cocos2d::Color4F::GREEN);
}
EditObjectFactory<PlayerObject> gPlayerFactory("Player", "playerStart", 5);


SpriteEditObject::SpriteEditObject(Editor* editor, const std::string& fileName)
	: EditObject(editor, "Sprite")
	, mFilename(fileName)
{
	mTypeName = "sprite";

	if (fileName.empty())
	{
		mNode = cocos2d::Sprite::create();
		mNode->setContentSize(cocos2d::Size(32, 32));
	}
	else
	{
		mNode = cocos2d::Sprite::create(fileName);
	}
}

void SpriteEditObject::parse(rapidjson::Value& obj)
{
	mFilename = obj["file"].GetString();
	float ax = obj["ax"].GetDouble();
	float ay = obj["ay"].GetDouble();

	cocos2d::Sprite* sprite = dynamic_cast<cocos2d::Sprite*>(mNode);
	sprite->initWithFile(mFilename);
	sprite->setAnchorPoint(cocos2d::Vec2(ax, ax));
}

void SpriteEditObject::serialize(rapidjson::Writer<rapidjson::FileWriteStream>& writer) const
{
	writer.String("file");
	writer.String(mFilename.c_str());

	writer.String("zOrder");
	writer.Int(mNode->getLocalZOrder());

	writer.String("ax");
	writer.Double(mNode->getAnchorPoint().x);
	writer.String("ay");
	writer.Double(mNode->getAnchorPoint().y);
}
//EditObjectFactory<SpriteObject> gSpriteFactory("Sprite", "sprite", 1);


BoxColliderObject::BoxColliderObject(Editor* editor, const std::string& className)
	: EditObject(editor, className)
{
	mNode = cocos2d::Node::create();
	mNode->setContentSize(cocos2d::Size(64, 64));
	mNode->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
}

void BoxColliderObject::draw()
{
	cocos2d::Rect rect = mNode->getBoundingBox();
	mEditor->mDrawNode->drawRect(rect.origin, rect.origin + rect.size, cocos2d::Color4F::GREEN);
}
EditObjectFactory<BoxColliderObject> gBoxColliderFactory("BoxCollider", "collider", 5);

PolygonColliderObject::PolygonColliderObject(Editor* editor, const std::string& className)
	: EditObject(editor, className)
{
	mNode = cocos2d::Node::create();
	mNode->setContentSize(cocos2d::Size(64, 64));
	mNode->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));

	mCrayonRenderer = std::make_unique<CrayonRenderer>(mNode);
}

bool PolygonColliderObject::handleTouch(const cocos2d::Vec2& pos)
{
	if (mCreateFinished)
		return false;

	if (mPoints.size() == 0)
	{
		mPoints.push_back(mNode->getPosition() - getPosition());
	}

	cocos2d::Vec2 relPos = pos - mNode->getPosition();

	if (mPoints.size() > 0)
	{
		if (mPoints[0].distanceSquared(relPos) <= 5.0f * 5.0f)
		{
			mPoints.push_back(mPoints[0]);

			computeCentroid();

			mCreateFinished = true;
			mIsPolygon = true;
			
			return false;
		}

		if (mPoints.back().distanceSquared(relPos) <= 5.0f * 5.0f)
		{
			computeCentroid();

			mCreateFinished = true;
			mIsPolygon = false;
			
			return false;
		}
	}

	mPoints.push_back(relPos);

	computeCentroid();

	mEditor->markDrawDirty();

	return true;
}

bool PolygonColliderObject::isCreatingFinished()
{
	return mCreateFinished;
}

void PolygonColliderObject::draw()
{
	if (mPoints.size() > 1)
	{
		for (size_t i = 1; i < mPoints.size(); ++i)
		{
			mEditor->mDrawNode->drawLine(mNode->getPosition() + mPoints[i - 1], mNode->getPosition() + mPoints[i], cocos2d::Color4F::BLUE);
		}
	}
}

void PolygonColliderObject::computeCentroid()
{
	if (mPoints.size() < 3)
		return;

	cocos2d::Vec2 min;
	cocos2d::Vec2 max;
	for (size_t i = 0; i < mPoints.size(); ++i)
	{
		if (mPoints[i].x < min.x)
			min.x = mPoints[i].x;
		if (mPoints[i].y < min.y)
			min.y = mPoints[i].y;

		if (mPoints[i].x > max.x)
			max.x = mPoints[i].x;
		if (mPoints[i].y > max.y)
			max.y = mPoints[i].y;
	}

	cocos2d::Vec2 mid = min.getMidpoint(max);
	mNode->setPosition(mNode->getPosition() + mid);

	cocos2d::Vec2 t = (max - min);
	mNode->setContentSize(cocos2d::Size(t.x, t.y));

	for (size_t i = 0; i < mPoints.size(); ++i)
	{
		mPoints[i] -= mid;
	}

	if (mPoints.size() >= 1)
	{
		min -= mid;
		mCrayonRenderer->clear();
		for (size_t i = 1; i < mPoints.size(); ++i)
		{
			mCrayonRenderer->drawSegment(mPoints[i - 1] - min, mPoints[i] - min);
		}
	}

	return;

	cocos2d::Vec2 center(0.f, 0.f);
	cocos2d::Vec2 ref(0.f, 0.f);
	float area = 0.f;
	const float inv3 = 1.0f / 3.0f;
	for (size_t i = 0; i < mPoints.size(); ++i)
	{
		cocos2d::Vec2 p1 = ref;
		cocos2d::Vec2 p2 = mPoints[i];
		cocos2d::Vec2 p3 = i + 1 < mPoints.size() ? mPoints[i + 1] : mPoints[0];

		cocos2d::Vec2 e1 = p2 - p1;
		cocos2d::Vec2 e2 = p3 - p1;
		float d = e1.cross(e2);

		float triangleArea = 0.5f * d;
		area += triangleArea;

		center += (p1 + p2 + p3) * triangleArea * inv3;
	}
	center *= 1.0f / area;

	mNode->setPosition(mNode->getPosition() + center);

	for (size_t i = 0; i < mPoints.size(); ++i)
	{
		mPoints[i] -= center;
	}
}

void PolygonColliderObject::parse(rapidjson::Value& obj)
{
	rapidjson::Value& points = obj["points"];
	if (points.IsArray())
	{
		for (rapidjson::SizeType i = 0; i < points.Size(); i++)
		{
			rapidjson::Value& pointVal = points[i];

			cocos2d::Vec2 point;
			point.x = pointVal["x"].GetDouble() * Physics::PPM;
			point.y = pointVal["y"].GetDouble() * Physics::PPM;
			mPoints.push_back(point);
		}
	}

	mCreateFinished = true;
}

void PolygonColliderObject::serialize(rapidjson::Writer<rapidjson::FileWriteStream>& writer) const
{
	writer.String("points");
	writer.StartArray();
	for (size_t i = 0; i < mPoints.size(); ++i)
	{
		writer.StartObject();

		writer.String("x");
		writer.Double(mPoints[i].x / Physics::PPM);
		writer.String("y");
		writer.Double(mPoints[i].y / Physics::PPM);

		writer.EndObject();
	}
	writer.EndArray();
}
EditObjectFactory<PolygonColliderObject> gPolygonColliderFactory("PolygonCollider", "chainCollider", 5);


KillZoneObject::KillZoneObject(Editor* editor, const std::string& className)
	: EditObject(editor, className)
{
	mNode = cocos2d::Node::create();
	mNode->setContentSize(cocos2d::Size(64, 64));
	mNode->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
}

void KillZoneObject::draw()
{
	cocos2d::Rect rect = mNode->getBoundingBox();
	mEditor->mDrawNode->drawRect(rect.origin, rect.origin + rect.size, cocos2d::Color4F::RED);
}
EditObjectFactory<KillZoneObject> gKillzoneFactory("KillZone", "killzone", 5);


CoinObject::CoinObject(Editor* editor, const std::string& className)
	: EditObject(editor, className)
{
	auto sprite = cocos2d::Sprite::create("Coin.png");
	mNode = sprite;
}
EditObjectFactory<CoinObject> gCoinFactory("Coin", "coin", 4);


MovingPlatformEditObject::MovingPlatformEditObject(Editor* editor, const std::string& className)
	: EditObject(editor, className)
{
	float width = 2.f;
	float height = 0.5f;
	

	mCrayonRenderer = std::make_unique<DynamicCrayonRenderer>();
	mNode = mCrayonRenderer->getNode();
	mNode->setContentSize(cocos2d::Size(64 * width, 64 * height));
	cocos2d::Rect rect = mNode->getBoundingBox();

	cocos2d::Vec2 origin = cocos2d::Vec2(rect.size / 2.f);
	mCrayonRenderer->drawSegment(cocos2d::Vec2(origin.x - 32.f * width, origin.y - 32.f * height), cocos2d::Vec2(origin.x - 32.f * width, origin.y + 32.f * height));
	mCrayonRenderer->drawSegment(cocos2d::Vec2(origin.x - 32.f * width, origin.y + 32.f * height), cocos2d::Vec2(origin.x + 32.f * width, origin.y + 32.f * height));
	mCrayonRenderer->drawSegment(cocos2d::Vec2(origin.x + 32.f * width, origin.y + 32.f * height), cocos2d::Vec2(origin.x + 32.f * width, origin.y - 32.f * height));
	mCrayonRenderer->drawSegment(cocos2d::Vec2(origin.x + 32.f * width, origin.y - 32.f * height), cocos2d::Vec2(origin.x - 32.f * width, origin.y - 32.f * height));

	mEndPos = cocos2d::Vec2(5.0f, 0.0f);
}
cocos2d::Vec2 MovingPlatformEditObject::getAnchorPoint() const
{
	return mEndPos;
}
void MovingPlatformEditObject::setAnchorPoint(const cocos2d::Vec2& point)
{
	mEndPos = point;
}
void MovingPlatformEditObject::parse(rapidjson::Value& obj)
{
	mEndPos.x = obj["ex"].GetDouble();
	mEndPos.y = obj["ey"].GetDouble();
}
void MovingPlatformEditObject::serialize(rapidjson::Writer<rapidjson::FileWriteStream>& writer) const
{
	writer.String("ex");
	writer.Double(mEndPos.x);
	writer.String("ey");
	writer.Double(mEndPos.y);
}
void MovingPlatformEditObject::draw()
{
	cocos2d::Vec2 destPos = mNode->getPosition() + mEndPos * 32.0f;
	mEditor->mDrawNode->drawLine(mNode->getPosition(), destPos, cocos2d::Color4F::YELLOW);
	mEditor->mDrawNode->drawDot(mNode->getPosition(), 2.0f, cocos2d::Color4F::YELLOW);
	mEditor->mDrawNode->drawDot(destPos, 2.0f, cocos2d::Color4F::YELLOW);
}
EditObjectFactory<MovingPlatformEditObject> gMovingPlatformFactory("MovingPlatform", "movingPlatform", 5);


LimitObject::LimitObject(Editor* editor, const std::string& className)
	: EditObject(editor, className)
{}

cocos2d::Vec2 LimitObject::getPosition()
{
	return cocos2d::Vec2(mEditor->mLevelWidth, mEditor->mLevelHeight);
}

void LimitObject::setPosition(const cocos2d::Vec2& pos)
{
	mEditor->mLevelWidth = pos.x;
	mEditor->mLevelHeight = pos.y;
}


EditObjectFactoryBase::EditObjectFactoryBase(const std::string& name, const std::string& type, int zOrder)
	: mName(name)
	, mType(type)
	, mZOrder(zOrder)
{
	EditObjectDatabase::registerObject(type, this);
}

void EditObjectDatabase::registerObject(const std::string& type, EditObjectFactoryBase* factory)
{
	getInstance()->mFactorys.insert(std::make_pair(type, factory));
}

EditObject* EditObjectDatabase::create(const std::string& type, Editor* editor)
{
	auto it = getInstance()->mFactorys.find(type);
	if (it != getInstance()->mFactorys.end())
	{
		EditObject* obj = it->second->create(editor);
		if (obj)
		{
			obj->mTypeName = it->second->mType;
		}
		return obj;
	}
	return 0;
}

EditObjectFactoryBase* EditObjectDatabase::getFactory(const std::string& type)
{
	auto it = getInstance()->mFactorys.find(type);
	if (it != getInstance()->mFactorys.end())
	{
		return it->second;
	}
	return 0;
}