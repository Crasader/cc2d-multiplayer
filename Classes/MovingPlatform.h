#ifndef __MOVINGPLATFORM_H__
#define __MOVINGPLATFORM_H__

#include "GameObject.h"

#include "TransformInterpolation.h"

class DynamicCrayonRenderer;

class MovingPlatform : public GameObject
{
public:
	MovingPlatform(GameSystem* game, const cocos2d::Vec2& pos, Parameters& spawnArgs);
	virtual ~MovingPlatform();

	virtual void update(float deltaTime) override;

	virtual std::uint32_t calcNetPriority(std::uint16_t clientId) override;
	virtual bool serialize(NetMessage& msg) override;

	cocos2d::Vec2 mStartPos;
	cocos2d::Vec2 mEndPos;

	bool mIsMovingToEnd = true;

	float mSpeed = 1.0f;

	TransformInterpolation mTransformInterpolation;

	std::unique_ptr<DynamicCrayonRenderer> mCrayonRenderer;
};

#endif