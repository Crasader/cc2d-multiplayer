#include "Parameters.h"

#include <sstream> 

Parameters::Parameters()
{

}

void Parameters::parse(rapidjson::Value& obj)
{
	auto it = obj.MemberBegin();
	auto end = obj.MemberEnd();
	while (it != end)
	{
		if (it->value.IsString())
			setString(it->name.GetString(), it->value.GetString());
		else if (it->value.IsBool())
			setBool(it->name.GetString(), it->value.GetBool());
		else if (it->value.IsInt())
			setInt(it->name.GetString(), it->value.GetInt());
		else if (it->value.IsDouble())
			setFloat(it->name.GetString(), it->value.GetDouble());
		else if (it->value.IsObject())
		{
			Parameters param;
			param.parse(it->value);
			setObject(it->name.GetString(), param);
		}
		else if (it->value.IsArray())
		{
			std::vector<Parameters> arrayParam;
			for (rapidjson::SizeType i = 0; i < it->value.Size(); i++)
			{
				Parameters param;
				param.parse(it->value[i]);
				arrayParam.push_back(param);
			}
			setArray(it->name.GetString(), arrayParam);
		}
		++it;
	}
}

bool Parameters::hasProp(const std::string& key)
{
	auto it = mPropMap.find(key);
	return (it != mPropMap.end());
}

void Parameters::setString(const std::string& key, const std::string& value)
{
	Value val;
	val.str = value;
	val.type = String;
	mPropMap[key] = val;
}

void Parameters::setBool(const std::string& key, bool value)
{
	std::stringstream ss;
	ss << value;

	Value val;
	val.str = ss.str();
	val.type = Bool;
	mPropMap[key] = val;
}

void Parameters::setInt(const std::string& key, int value)
{
	std::stringstream ss;
	ss << value;

	Value val;
	val.str = ss.str();
	val.type = Int;
	mPropMap[key] = val;
}

void Parameters::setFloat(const std::string& key, float value)
{
	std::stringstream ss;
	ss << value;

	Value val;
	val.str = ss.str();
	val.type = Float;
	mPropMap[key] = val;
}

void Parameters::setObject(const std::string& key, Parameters& param)
{
	std::stringstream ss;
	ss << mObjects.size();

	Value val;
	val.str = ss.str();
	val.type = Object;
	mPropMap[key] = val;

	mObjects.push_back(param);
}

void Parameters::setArray(const std::string& key, std::vector<Parameters>& param)
{
	std::stringstream ss;
	ss << mArray.size();

	Value val;
	val.str = ss.str();
	val.type = Array;
	mPropMap[key] = val;

	mArray.push_back(param);
}

std::string Parameters::getString(const std::string& key, const std::string& defaultValue)
{
	auto it = mPropMap.find(key);
	if (it != mPropMap.end() && it->second.type == String)
	{
		return it->second.str;
	}
	return defaultValue;
}

bool Parameters::getBool(const std::string& key, bool defaultValue)
{
	auto it = mPropMap.find(key);
	if (it != mPropMap.end() && it->second.type == Bool)
	{
		bool value;
		std::stringstream ss(it->second.str);
		ss >> value;
		return value;
	}
	return defaultValue;
}

int Parameters::getInt(const std::string& key, int defaultValue)
{
	auto it = mPropMap.find(key);
	if (it != mPropMap.end() && it->second.type == Int)
	{
		int value;
		std::stringstream ss(it->second.str);
		ss >> value;
		return value;
	}
	return defaultValue;
}

float Parameters::getFloat(const std::string& key, float defaultValue)
{
	auto it = mPropMap.find(key);
	if (it != mPropMap.end() && it->second.type == Float)
	{
		float value;
		std::stringstream ss(it->second.str);
		ss >> value;
		return value;
	}
	return defaultValue;
}

Parameters Parameters::getObject(const std::string& key)
{
	auto it = mPropMap.find(key);
	if (it != mPropMap.end() && it->second.type == Object)
	{
		int value;
		std::stringstream ss(it->second.str);
		ss >> value;

		return mObjects[value];
	}
	return Parameters();
}

std::vector<Parameters> Parameters::getArray(const std::string& key)
{
	auto it = mPropMap.find(key);
	if (it != mPropMap.end() && it->second.type == Array)
	{
		int value;
		std::stringstream ss(it->second.str);
		ss >> value;

		return mArray[value];
	}
	return std::vector<Parameters>();
}

bool Parameters::getString(const std::string& key, std::string& value, const std::string& defaultValue)
{
	auto it = mPropMap.find(key);
	if (it != mPropMap.end() && it->second.type == String)
	{
		value = it->second.str;
		return true;
	}
	value = defaultValue;
	return false;
}

bool Parameters::getBool(const std::string& key, bool& value, bool defaultValue)
{
	auto it = mPropMap.find(key);
	if (it != mPropMap.end() && it->second.type == Bool)
	{
		std::stringstream ss(it->second.str);
		ss >> value;
		return true;
	}
	value = defaultValue;
	return false;
}

bool Parameters::getInt(const std::string& key, int& value, int defaultValue)
{
	auto it = mPropMap.find(key);
	if (it != mPropMap.end() && it->second.type == Int)
	{
		std::stringstream ss(it->second.str);
		ss >> value;
		return true;
	}
	value = defaultValue;
	return false;
}

bool Parameters::getFloat(const std::string& key, float& value, float defaultValue)
{
	auto it = mPropMap.find(key);
	if (it != mPropMap.end() && it->second.type == Float)
	{
		std::stringstream ss(it->second.str);
		ss >> value;
		return true;
	}
	value = defaultValue;
	return false;
}

bool Parameters::getObject(const std::string& key, Parameters& param)
{
	auto it = mPropMap.find(key);
	if (it != mPropMap.end() && it->second.type == Array)
	{
		int value;
		std::stringstream ss(it->second.str);
		ss >> value;

		param = mObjects[value];

		return true;
	}
	return false;
}

bool Parameters::getArray(const std::string& key, std::vector<Parameters>& param)
{
	auto it = mPropMap.find(key);
	if (it != mPropMap.end() && it->second.type == Array)
	{
		int value;
		std::stringstream ss(it->second.str);
		ss >> value;

		param = mArray[value];

		return true;
	}
	return false;
}