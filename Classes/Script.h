#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include <string>
#include <vector>
#include "angelscript.h"

#include "GameObject.h"

class ScriptManager;

class Script
{
public:
	Script(ScriptManager* mgr, const std::string& name);
	virtual ~Script();

	void clear();

	bool createFromFile(const std::string& file);

	ScriptFunction* getOrCreateFunction(const std::string& funcName);

	asIScriptContext* getContext() const { return mContext; }
	asIScriptModule* getModule() const { return mModule; }
	std::string getModuleName() const { return mModuleName; }

	bool call(const std::string& funcName);

	template <class... Args>
	bool call(const std::string& funcName, Args... params)
	{
		asIScriptFunction* asFunc = mModule->GetFunctionByName(funcName.c_str());
		if (!asFunc)
			return false;

		return call(asFunc, params...);
	}
	template <class... Args>
	bool call(asIScriptFunction* func, Args... params)
	{
		asIScriptContext* ctx = prepareContextFromPool(func);
		if (!ctx)
			return false;

		mContext = ctx;
		SetArg(0, params...);

		int r = executeCall();

		if (func->GetReturnTypeId() == asTYPEID_VOID)
			returnContextToPool(ctx);

		return (r == asEXECUTION_FINISHED);
	}
	template <class T, class... Args>
	void SetArg(int index, T value, Args... params)
	{
		mContext->SetArgAddress(index, &value);
		SetArg(index + 1, params...);
	}
	template <class... Args>
	void SetArg(int index, int value, Args... params)
	{
		mContext->SetArgDWord(index, value);
		SetArg(index + 1, params...);
	}
	template <class... Args>
	void SetArg(int index, unsigned int value, Args... params)
	{
		mContext->SetArgDWord(index, value);
		SetArg(index + 1, params...);
	}
	template <class... Args>
	void SetArg(int index, short value, Args... params)
	{
		mContext->SetArgWord(index, value);
		SetArg(index + 1, params...);
	}
	template <class... Args>
	void SetArg(int index, unsigned short value, Args... params)
	{
		mContext->SetArgWord(index, value);
		SetArg(index + 1, params...);
	}
	template <class... Args>
	void SetArg(int index, float value, Args... params)
	{
		mContext->SetArgFloat(index, value);
		SetArg(index + 1, params...);
	}
	template <class... Args>
	void SetArg(int index, GameObject* value, Args... params)
	{
		if (value)
		{
			mContext->SetArgDWord(index, value->getLocalIndex());
		}
		else
			mContext->SetArgDWord(index, std::numeric_limits<std::uint32_t>::max());
		SetArg(index + 1, params...);
	}
	void SetArg(int index)
	{
	}

	int prepare(asIScriptFunction* func);
	int executeCall();

	asIScriptContext* prepareContextFromPool(asIScriptFunction* func);
	void returnContextToPool(asIScriptContext* ctx);

	void finsishCall();

protected:
	ScriptManager* mScriptManager = nullptr;

	asIScriptContext* mContext = nullptr;
	asIScriptModule* mModule = nullptr;
	std::string mModuleName;

	std::map<std::string, std::unique_ptr<ScriptFunction>> mScriptFunctions;
};

#endif