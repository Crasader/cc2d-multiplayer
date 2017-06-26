#include "CrayonPolygon.h"

#include "GameSystem.h"
#include "Physics.h"
#include "LevelScene.h"

#include "DynamicCrayonRenderer.h"

CrayonPolygon::CrayonPolygon(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs)
	: GameObject(game)
{
	float width = spawnArgs.getFloat("sx", 1.f);
	float height = spawnArgs.getFloat("sy", 1.f);

	mCrayonRenderer = std::make_unique<DynamicCrayonRenderer>();
	mNode = mCrayonRenderer->getNode();
	setPosition(pos);
	game->addNode(mNode, 5);

	std::vector<b2Vec2> points;
	if (spawnArgs.hasProp("points"))
	{
		bool isLoop = spawnArgs.getBool("loop", false);

		b2BodyDef bodyDef;
		bodyDef.position.Set(pos.x, pos.y);
		bodyDef.type = b2BodyType::b2_staticBody;
		mBody = mGame->mPhysics->world->CreateBody(&bodyDef);

		b2Filter filter;
		filter.categoryBits = CollisionGroup::BOUNDS;
		filter.maskBits = CollisionGroup::ALL;

		b2ChainShape chainShape;
		std::vector<Parameters> arr;
		spawnArgs.getArray("points", arr);
		for (size_t i = 0; i < arr.size(); ++i)
		{
			float x = arr[i].getFloat("x");
			float y = arr[i].getFloat("y");
			points.emplace_back(x, y);
		}

		if (isLoop)
		{
			chainShape.CreateLoop(&points[0], points.size() - 1);
		}
		else
		{
			chainShape.CreateChain(&points[0], points.size());
		}

		b2FixtureDef fd;
		fd.shape = &chainShape;
		fd.density = 0.0f;
		fd.friction = 0.6f;
		fd.filter = filter;
		fd.userData = this;
		mBody->CreateFixture(&fd);
	}

	for (size_t i = 1; i < points.size(); ++i)
	{
		mCrayonRenderer->drawSegment(Box2D2cocos2(points[i - 1]) * Physics::PPM, Box2D2cocos2(points[i]) * Physics::PPM);
	}
}

CrayonPolygon::~CrayonPolygon()
{

}

#include "GameObjectFactory.h"
REGISTER_GAMEOBJECT(CrayonPolygon, GameObject::OT_CrayonPolygon, "crayonPolygon", false);


#include "EditorObjects.h"
#include "Editor.h"

class CrayonPolygonEditObject : public EditObject
{
public:
	CrayonPolygonEditObject(Editor* editor, const std::string& className)
		: EditObject(editor, className)
	{
		mCrayonRenderer = std::make_unique<DynamicCrayonRenderer>();

		mNode = mCrayonRenderer->getNode();
		mNode->setContentSize(cocos2d::Size(64.f, 64.f));
	}

	virtual bool canScale() { return false; }

	void draw()
	{
		cocos2d::Rect rect = mNode->getBoundingBox();

		if (mPoints.size() > 1)
		{
			for (size_t i = 1; i < mPoints.size(); ++i)
			{
				mEditor->mDrawNode->drawLine(mNode->getPosition() + mPoints[i - 1], mNode->getPosition() + mPoints[i], cocos2d::Color4F::BLUE);
			}
		}

		cocos2d::Vec2 offset = cocos2d::Vec2(rect.size / 2.f);
		mCrayonRenderer->clear();
		for (size_t i = 1; i < mPoints.size(); ++i)
		{
			mCrayonRenderer->drawSegment(offset + mPoints[i - 1], offset + mPoints[i]);
		}
	}

	bool isCreatingFinished()
	{
		return mCreateFinished;
	}

	bool handleTouch(const cocos2d::Vec2& pos)
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
			if (mPoints[0].distanceSquared(relPos) <= 20.0f * 20.0f)
			{
				mPoints.push_back(mPoints[0]);

				computeCentroid();

				mCreateFinished = true;
				mIsPolygon = true;

				return false;
			}

			if (mPoints.back().distanceSquared(relPos) <= 20.0f * 20.0f)
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

	void computeCentroid()
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
	}

	void parse(rapidjson::Value& obj)
	{
		mIsPolygon = obj["loop"].GetBool();

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

		computeCentroid();

		mCreateFinished = true;
	}

	void serialize(rapidjson::Writer<rapidjson::FileWriteStream>& writer) const
	{
		writer.String("loop");
		writer.Bool(mIsPolygon);

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

	std::unique_ptr<DynamicCrayonRenderer> mCrayonRenderer;
	std::vector<cocos2d::Vec2> mPoints;
	bool mCreateFinished = false;
	bool mIsPolygon = false;
};
EditObjectFactory<CrayonPolygonEditObject> gTargetPointFactory("CrayonPolygon", "crayonPolygon", 5);