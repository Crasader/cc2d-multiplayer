#ifndef __CHARTPANEL_H__
#define __CHARTPANEL_H__

#include "cocos2d.h"

class ChartPanel
{
	struct Point
	{
		float value;
		cocos2d::Color4F color;
	};

public:
	ChartPanel(cocos2d::Node* node);
	~ChartPanel();

	size_t createPoints(cocos2d::Color4F color = cocos2d::Color4F::RED);
	void addPoint(float y, cocos2d::Color4F color = cocos2d::Color4F::RED, size_t index = 0);

	void draw();

	float mChartWidth = 800.f;
	float mChartHeight = 300.f;

	float mYAxisMax = 1.f;

	cocos2d::DrawNode* mDrawNode = nullptr;

	std::vector<std::vector<Point>> mData;
};

#endif