#include "ChartPanel.h"

ChartPanel::ChartPanel(cocos2d::Node* node)
{
	mDrawNode = cocos2d::DrawNode::create();
	node->addChild(mDrawNode, 10);
}

ChartPanel::~ChartPanel()
{

}

size_t ChartPanel::createPoints(cocos2d::Color4F color)
{
	size_t index = mData.size();

	std::vector<Point> points;
	points.resize(101);
	for (size_t i = 0; i < 101; ++i)
	{
		points[i].value = 0.f;
		points[i].color = color;
	}
	mData.push_back(points);

	return index;
}

void ChartPanel::addPoint(float y, cocos2d::Color4F color, size_t index)
{
	if (index >= mData.size())
	{
		return;
	}

	std::vector<Point>& points = mData[index];

	size_t i = 0;
	for (i = 0; i < points.size() - 1; ++i)
	{
		points[i] = points[i + 1];
	}
	points[i].value = y;
}

void ChartPanel::draw()
{
	mDrawNode->clear();
	mDrawNode->drawRect(cocos2d::Vec2(0, 0), cocos2d::Vec2(mChartWidth, mChartHeight), cocos2d::Color4F::BLUE);

	float xStep = mChartWidth / (101 - 1);
	for (size_t i = 1; i < 101 - 1; ++i)
	{
		float xPos = 0 + (xStep * i);
		mDrawNode->drawLine(cocos2d::Vec2(xPos, 0), cocos2d::Vec2(xPos, mChartHeight), cocos2d::Color4F(0, 0, 1, 0.5f));
	}

	float yStep = mChartHeight / 4;
	for (size_t i = 1; i < 4; ++i)
	{
		float yPos = 0 + (yStep * i);
		mDrawNode->drawLine(cocos2d::Vec2(0, yPos), cocos2d::Vec2(mChartWidth, yPos), cocos2d::Color4F(0, 0, 1, 0.5f));
	}

	for (size_t i = 0; i < mData.size(); ++i)
	{
		std::vector<Point>& points = mData[i];
		for (size_t pIdx = 1; pIdx < points.size(); ++pIdx)
		{
			float lastXPos = 0 + (xStep * (pIdx - 1));
			float lastYPos = mChartHeight * (points[pIdx - 1].value / mYAxisMax);

			float xPos = 0 + (xStep * pIdx);
			float yPos = mChartHeight * (points[pIdx].value / mYAxisMax);

			if (pIdx >= 0)
			{
				mDrawNode->drawLine(cocos2d::Vec2(lastXPos, lastYPos), cocos2d::Vec2(xPos, yPos), points[pIdx].color);
			}
		}
	}
}