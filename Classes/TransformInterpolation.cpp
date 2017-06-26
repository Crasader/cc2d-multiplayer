#include "TransformInterpolation.h"

#include "ChartPanel.h"

TransformInterpolation::TransformInterpolation()
{
	
}

TransformInterpolation::~TransformInterpolation()
{

}

void TransformInterpolation::init(size_t max, float positionThreshold)
{
	if (max > 0)
	{
		mTransforms.resize(max);

		mSqrPositionThreshold = positionThreshold * positionThreshold;
	}

	reset();
}

void TransformInterpolation::reset()
{
	mTail = 0;
	mHead = 0;
}

void TransformInterpolation::add(std::uint32_t time, const cocos2d::Vec2& pos, const cocos2d::Vec2& vel, float rot)
{
	if (mTransforms.size() == 0)
		return;

	Transform& trans = mTransforms[mHead];
	trans.time = time;
	trans.pos = pos;
	trans.vel = vel;
	trans.rot = rot;

	mHead = next(mHead);
	if (mHead == mTail)
	{
		mTail = next(mTail);
	}
}

bool TransformInterpolation::interpolate(std::uint32_t currTime, cocos2d::Vec2* pos, cocos2d::Vec2* vel, float* rot)
{
	if (mTransforms.size() == 0 || mHead == mTail)
		return false;

	std::uint32_t front = prev(mHead);
	std::uint32_t index = front;
	while (index != mTail)
	{
		const Transform& trans = mTransforms[index];
		if (currTime >= trans.time)
		{
			std::int32_t timeDiff = currTime - trans.time;
			if (timeDiff < 0)
				timeDiff = 0;

			if (timeDiff > 250)
			{
				timeDiff = 250;
			}
			if (index == front)
			{
				float rate = (1.f / 30.f);// 0.001f
				if (index != mTail)
				{
					const Transform& prevTrans = mTransforms[prev(index)];
					rate = (trans.time - prevTrans.time) * 0.001f;
				}

				// Extrapolate
				float extrapolateTime = timeDiff * 0.001f;/*/ rate*/;

				cocos2d::Vec2 velocity;
				if (vel)
				{
					velocity = trans.vel;
					*vel = trans.vel;
				}
				else if (index != mTail)
				{
					const Transform& prevTrans = mTransforms[prev(index)];
					velocity = (trans.pos - prevTrans.pos);
				}

				if (pos)
					*pos = trans.pos + velocity * extrapolateTime;
				if (rot)
					*rot = trans.rot;
			}
			else
			{
				const Transform& nextTrans = mTransforms[next(index)];

				if (pos)
				{
					float sqDist = nextTrans.pos.distanceSquared(*pos);
					if (sqDist >= mSqrPositionThreshold)
					{
						*pos = nextTrans.pos;
						if (vel)
							*vel = nextTrans.vel;
						if (rot)
							*rot = nextTrans.rot;

						mTail = next(index);
						return false;
					}
				}

				float delta = (float)(nextTrans.time - trans.time);
				float lerp = (float)(currTime - trans.time) / delta;

				if (mChartPanel)
					mChartPanel->addPoint(lerp, cocos2d::Color4F::BLUE, mChartIndex);

				lerp = std::max(0.f, std::min(1.f, lerp));

				if (pos)
					*pos = trans.pos + (nextTrans.pos - trans.pos) * lerp;
				if (vel)
					*vel = trans.vel + (nextTrans.vel - trans.vel) * lerp;
				if (rot)
					*rot = trans.rot + (nextTrans.rot - trans.rot) * lerp;
			}

			return true;
		}

		index = prev(index);
	}

	const Transform& trans = mTransforms[mTail];
	if (pos)
		*pos = trans.pos;
	if (vel)
		*vel = trans.vel;
	if (rot)
		*rot = trans.rot;

	return false;
}

const TransformInterpolation::Transform& TransformInterpolation::newest()
{
	std::uint32_t index = prev(mHead);
	return mTransforms[index];
}

const TransformInterpolation::Transform& TransformInterpolation::oldest()
{
	return mTransforms[mTail];
}

std::uint32_t TransformInterpolation::next(std::uint32_t index)
{
	index++;
	if (index >= mTransforms.size())
		index = 0;
	return index;
}

std::uint32_t TransformInterpolation::prev(std::uint32_t index)
{
	if (index == 0)
		index = mTransforms.size() - 1;
	else
		index--;
	return index;
}