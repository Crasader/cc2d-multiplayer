#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

#include <map>
#include <vector>
#include "json/document.h"

class Parameters
{
public:
	enum eType
	{
		None,
		String,
		Bool,
		Int,
		Float,
		Object,
		Array
	};

	struct Value
	{
		std::string str;
		eType type;
	};

public:
	Parameters();

	void parse(rapidjson::Value& obj);

	bool hasProp(const std::string& key);

	void setString(const std::string& key, const std::string& value);
	void setBool(const std::string& key, bool value);
	void setInt(const std::string& key, int value);
	void setFloat(const std::string& key, float value);
	void setObject(const std::string& key, Parameters& param);
	void setArray(const std::string& key, std::vector<Parameters>& param);

	std::string getString(const std::string& key, const std::string& defaultValue = "");
	bool getBool(const std::string& key, bool defaultValue = false);
	int getInt(const std::string& key, int defaultValue = 0);
	float getFloat(const std::string& key, float defaultValue = 0);
	Parameters getObject(const std::string& key);
	std::vector<Parameters> getArray(const std::string& key);

	bool getString(const std::string& key, std::string& value, const std::string& defaultValue);
	bool getBool(const std::string& key, bool& value, bool defaultValue);
	bool getInt(const std::string& key, int& value, int defaultValue);
	bool getFloat(const std::string& key, float& value, float defaultValue);
	bool getObject(const std::string& key, Parameters& param);
	bool getArray(const std::string& key, std::vector<Parameters>& param);

//private:
	std::map<std::string, Value> mPropMap;
	std::vector<Parameters> mObjects;
	std::vector<std::vector<Parameters>> mArray;
};

#endif