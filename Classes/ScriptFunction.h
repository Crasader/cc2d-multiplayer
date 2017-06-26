#ifndef __SCRIPTFUNCTION_H__
#define __SCRIPTFUNCTION_H__

class Script;
class asIScriptFunction;

class ScriptFunction
{
public:
	ScriptFunction(Script* script, asIScriptFunction* func)
		: mScript(script)
		, mFunc(func)
	{}
	virtual ~ScriptFunction()
	{}

	template <class... Args>
	void call(Args... params)
	{
		if (!mFunc)
			return;
		mScript->call(mFunc, params...);
	}

	Script* mScript = nullptr;
	asIScriptFunction* mFunc = nullptr;
};

#endif