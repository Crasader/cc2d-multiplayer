#ifndef __ROPE_H__
#define __ROPE_H__

#include "GameObject.h"

#include "2d/CCDrawNode.h"

class DynamicCrayonRenderer;

class Rope : public GameObject
{
public:
	Rope(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs);
	virtual ~Rope();

	void createRope(GameObject* attachedObject1, GameObject* attachedObject2, const cocos2d::Vec2& startPos, const cocos2d::Vec2& endPos);
	void destroyRope();

	void update(float deltaTime) override;

	std::vector<b2Body*> mRopeParts;
	std::vector<b2Joint*> mJoints;

	GameObject* mAttachedObject1 = nullptr;
	GameObject* mAttachedObject2 = nullptr;

	cocos2d::Vec2 mStartPos;
	cocos2d::Vec2 mEndPos;

	std::unique_ptr<DynamicCrayonRenderer> mCrayonRenderer;
};

#endif