#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include "Parameters.h"

#include "2d\CCNode.h"
#include "base\CCRefPtr.h"

#include "Box2D/Dynamics/b2Body.h"

#include "Delegate.h"

#include "NetObject.h"

class GameSystem;
class NetMessage;
struct ContactInfo;

class EventData
{
public:
	EventData()
	{}
	virtual ~EventData()
	{}
};

class ScriptFunctionEvent : public EventData
{
public:
	std::string eventName;
	std::string funcName;
};

class GameObject : public NetObject
{
public:
	enum eObjectType
	{
		OT_None,
		OT_PlayerStart,
		OT_Player,
		OT_Sprite,
		OT_Coin,
		OT_TriggerBox,
		OT_KillZone,
		OT_TargetPoint,
		OT_Collider,
		OT_ChainCollider,
		OT_CrayonBox,
		OT_CrayonPolygon,
		OT_MovingPlatform,
		OT_Ball,
		OT_StickyBomb,
		OT_Rope
	};

public:
	GameObject(GameSystem* game);
	virtual ~GameObject();

	virtual void onNetDestroy();

	const std::string& getName() const { return mName; };

	std::uint32_t getLocalIndex() const { return mLocalIndex; }

	void destroyMe();

	virtual const cocos2d::Vec2 getPosition() const;
	virtual void setPosition(const cocos2d::Vec2& pos);

	virtual float getRotation() const;
	virtual void setRotation(float degree);

	virtual void handleEvent(const EventData* evt);

	virtual void onContactEnter(GameObject* obj, const ContactInfo& contact);
	virtual void onContactLeave(GameObject* obj, const ContactInfo& contact);
	virtual void onContactPreSolve(b2Contact* contact, const b2Manifold* oldManifold) {}
	virtual void onContactPostSolve(b2Contact* contact, const struct b2ContactImpulse* impulse) {}

	virtual void onTakeDamage(GameObject* attacker, float amount, const cocos2d::Vec2& hitPos) {}

	virtual void update(float deltaTime);

	virtual void writeNetCreateInit(NetMessage& msg) override;

	GameSystem* mGame = nullptr;
	cocos2d::RefPtr<cocos2d::Node> mNode;
	b2Body* mBody = nullptr;

	std::string mName;

	std::uint32_t mLocalIndex;
	bool mIsDestroyed = false;

	float mMaxVelocity = 20.f;

	MulticastDelegate<GameObject*> mOnDestroyed;
	MulticastDelegate<GameObject*, GameObject*, const ContactInfo&> mOnContactEnter;
	MulticastDelegate<GameObject*, GameObject*, const ContactInfo&> mOnContactLeave;
};

#endif