#include "Script.h"

#include "ScriptManager.h"

Script::Script(ScriptManager* mgr, const std::string& name)
	: mScriptManager(mgr)
	, mModuleName(name)
{
}

Script::~Script()
{
	mScriptManager->getEngine()->DiscardModule(mModuleName.c_str());
}

void Script::clear()
{
	mScriptFunctions.clear();
}

bool Script::createFromFile(const std::string& file)
{
	clear();

	FILE *f = fopen(file.c_str(), "rb");
	if (f == 0)
	{
		return false;
	}

	// Determine the size of the file	
	fseek(f, 0, SEEK_END);
	int len = ftell(f);
	fseek(f, 0, SEEK_SET);

	std::string script;
	script.resize(len);
	size_t c = fread(&script[0], len, 1, f);
	fclose(f);

	mModule = mScriptManager->getEngine()->GetModule(mModuleName.c_str(), asGM_ALWAYS_CREATE);
	if (!mModule)
		return false;

	if (mModule->AddScriptSection(file.c_str(), script.c_str(), script.size()) < 0)
	{
		return false;
	}

	if (mModule->Build() < 0)
	{
		return false;
	}

	std::string n = mModule->GetName();

	return true;
}

ScriptFunction* Script::getOrCreateFunction(const std::string& funcName)
{
	if (!mModule)
		return nullptr;

	auto it = mScriptFunctions.find(funcName);
	if (it != mScriptFunctions.end())
	{
		return it->second.get();
	}

	asIScriptFunction* asFunc = mModule->GetFunctionByName(funcName.c_str());
	if (!asFunc)
		return nullptr;

	auto func = std::make_unique<ScriptFunction>(this, asFunc);
	ScriptFunction* funcPtr = func.get();
	mScriptFunctions.insert(std::make_pair(funcName, std::move(func)));

	return funcPtr;
}

bool Script::call(const std::string& funcName)
{
	if (!mModule)
		return false;

	asIScriptFunction* asFunc = mModule->GetFunctionByName(funcName.c_str());
	if (!asFunc)
		return false;

	asIScriptContext *ctx = mScriptManager->prepareContextFromPool(asFunc);
	if (!ctx)
		return false;

	mContext = ctx;
	int r = executeCall();

	if (asFunc->GetReturnTypeId() == asTYPEID_VOID)
		mScriptManager->returnContextToPool(ctx);

	return (r == asEXECUTION_FINISHED);
}

int Script::prepare(asIScriptFunction* func)
{
	return mContext->Prepare(func);
}

int Script::executeCall()
{
	int r = mContext->Execute();
	if (r != asEXECUTION_FINISHED)
	{
		if (r == asEXECUTION_EXCEPTION)
		{
			std::string exception = mContext->GetExceptionString();
			std::string function = mContext->GetExceptionFunction()->GetDeclaration();
			int line = mContext->GetExceptionLineNumber();
		}
	}
	return r;
}

void Script::finsishCall()
{
	returnContextToPool(mContext);
}

asIScriptContext* Script::prepareContextFromPool(asIScriptFunction* func)
{
	return mScriptManager->prepareContextFromPool(func);
}

void Script::returnContextToPool(asIScriptContext* ctx)
{
	if (mContext == ctx)
		mContext = nullptr;
	mScriptManager->returnContextToPool(ctx);
}