#ifndef __TRANSFORMINTERPOLATION_H__
#define __TRANSFORMINTERPOLATION_H__

#include <cstdint>
#include <vector>

#include "math/Vec2.h"

class TransformInterpolation
{
public:
	struct Transform
	{
		std::uint32_t time;
		cocos2d::Vec2 pos;
		cocos2d::Vec2 vel;
		float rot;
	};

public:
	TransformInterpolation();
	~TransformInterpolation();

	bool isEmpty() const { return mHead == mTail; }

	void init(size_t max = 0, float positionThreshold = 2.f);

	void reset();

	void add(std::uint32_t time, const cocos2d::Vec2& pos, const cocos2d::Vec2& vel, float rot);

	bool interpolate(std::uint32_t currTime, cocos2d::Vec2* pos, cocos2d::Vec2* vel, float* rot);

	const Transform& newest();
	const Transform& oldest();

	std::uint32_t next(std::uint32_t index);
	std::uint32_t prev(std::uint32_t index);

	std::vector<Transform> mTransforms;
	size_t mTail = 0;
	size_t mHead = 0;

	float mSqrPositionThreshold = 0.f;

	class ChartPanel* mChartPanel = nullptr;
	size_t mChartIndex;
};

template <class T>
class StateHistory
{
public:
	StateHistory()
	{}
	~StateHistory()
	{}

	bool empty() const { return mStates.empty() || mHead == mTail; }

	void reset()
	{
		mTail = 0;
		mHead = 0;
	}

	void init(size_t max = 0)
	{
		if (max > 0)
		{
			mStates.resize(max);
		}

		reset();
	}

	void add(std::uint32_t time, const T& state)
	{
		if (mStates.size() == 0)
			return;

		mStates[mTail] = state;
		mStates[mTail].time = time;	

		mTail = next(mTail);
		if (mTail == mHead)
		{
			mHead = next(mHead);
		}
	}

	bool getState(std::uint32_t currTime, T& state)
	{
		if (mStates.size() == 0 || mHead == mTail)
			return false;

		std::uint32_t tail = prev(mTail);
		std::uint32_t index = tail;
		while (index != mHead)
		{
			const T& currState = mStates[index];

			if (currTime >= currState.time)
			{
				state = currState;

				return true;
			}

			index = prev(index);
		}
		return false;
	}

	const T& newest()
	{
		std::uint32_t index = prev(mTail);
		return mStates[index];
	}

	std::uint32_t next(std::uint32_t index)
	{
		index++;
		if (index >= mStates.size())
			index = 0;
		return index;
	}
	std::uint32_t prev(std::uint32_t index)
	{
		if (index == 0)
			index = mStates.size() - 1;
		else
			index--;
		return index;
	}

	std::vector<T> mStates;
	size_t mTail = 0;
	size_t mHead = 0;
};

#endif