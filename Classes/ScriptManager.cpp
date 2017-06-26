#include "ScriptManager.h"

#include <string>

#include "Script.h"

#include "scriptstdstring.h"
#include "scriptarray.h"

#include "cocos2d.h"

#include "GameSystem.h"
#include "NetworkManager.h"
#include "Physics.h"
#include "Input.h"
#include "GameMode.h"
#include "GameObject.h"
#include "Player.h"


#include "Parameters.h"

int ScriptObject::AddRef()
{
	return ++refCount;
}

int ScriptObject::Release()
{
	if (--refCount == 0)
	{
		//delete this;
		mScriptManager->removeScriptObject(this);
		return 0;
	}
	return refCount;
}

std::uint32_t ScriptObject::getLocalIndex() const
{
	return mObject->getLocalIndex();
}

std::uint32_t ScriptObject::getType() const
{
	return mObject->mTypeId;
}

cocos2d::Vec2 ScriptObject::getPosition()
{
	return mObject->getPosition();
}

void print(std::string& msg)
{
	cocos2d::MessageBox(msg.c_str(), "Test");
}

static void Vec2Constructor(cocos2d::Vec2 *self)
{
	new(self)cocos2d::Vec2();
}

static void ParametersConstructor(Parameters *self)
{
	new(self)Parameters();
}

static void ContactInfoConstructor(ContactInfo *self)
{
	new(self)ContactInfo();
}

static void NetMessageConstructor(ContactInfo *self)
{
	new(self)ContactInfo();
}



ScriptManager::ScriptManager(GameSystem* game)
	: mGame(game)
{
	mEngine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	if (!mEngine)
	{
		return;
	}

	int r = mEngine->SetMessageCallback(asMETHOD(ScriptManager, messageCallback), this, asCALL_THISCALL);

	RegisterStdString(mEngine);
	RegisterScriptArray(mEngine, true);

	r = mEngine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(print), asCALL_CDECL);

	r = mEngine->RegisterGlobalProperty("int gMaxConnections", &mGame->mNetwork->mMaxConnections); assert(r >= 0);

	r = mEngine->RegisterGlobalProperty("uint16 gInvalidClientId", &mGame->mNetwork->InvalidClientId); assert(r >= 0);

	r = mEngine->RegisterGlobalProperty("float gLevelHeight", &mGame->mLevelHeight); assert(r >= 0);
	r = mEngine->RegisterGlobalProperty("float gLevelWidth", &mGame->mLevelWidth); assert(r >= 0);

	mEngine->RegisterEnum("ObjectType");
	mEngine->RegisterEnumValue("ObjectType", "OT_None", 0);
	mEngine->RegisterEnumValue("ObjectType", "OT_PlayerStart", 1);
	mEngine->RegisterEnumValue("ObjectType", "OT_Player", 2);
	mEngine->RegisterEnumValue("ObjectType", "OT_Sprite", 3);
	mEngine->RegisterEnumValue("ObjectType", "OT_Coin", 4);
	mEngine->RegisterEnumValue("ObjectType", "OT_TriggerBox", 5);
	mEngine->RegisterEnumValue("ObjectType", "OT_KillZone", 6);
	mEngine->RegisterEnumValue("ObjectType", "OT_TargetPoint", 7);
	mEngine->RegisterEnumValue("ObjectType", "OT_Collider", 8);
	mEngine->RegisterEnumValue("ObjectType", "OT_ChainCollider", 9);
	mEngine->RegisterEnumValue("ObjectType", "OT_CrayonBox", 10);
	mEngine->RegisterEnumValue("ObjectType", "OT_CrayonPolygon", 11);
	mEngine->RegisterEnumValue("ObjectType", "OT_MovingPlatform", 12);
	mEngine->RegisterEnumValue("ObjectType", "OT_Ball", 13);
	mEngine->RegisterEnumValue("ObjectType", "OT_StickyBomb", 14);
	mEngine->RegisterEnumValue("ObjectType", "OT_Rope", 15);


	r = mEngine->RegisterObjectType("Vec2", sizeof(cocos2d::Vec2), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK);
	r = mEngine->RegisterObjectProperty("Vec2", "float x", offsetof(cocos2d::Vec2, x));
	r = mEngine->RegisterObjectProperty("Vec2", "float y", offsetof(cocos2d::Vec2, y));
	r = mEngine->RegisterObjectBehaviour("Vec2", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Vec2Constructor), asCALL_CDECL_OBJLAST);
	//r = mEngine->RegisterObjectBehaviour("Vec2", asBEHAVE_CONSTRUCT, "void f(float, float)", asFUNCTION(OgreVector3InitConstructor), asCALL_CDECL_OBJLAST);


	r = mEngine->RegisterObjectType("Parameters", sizeof(Parameters), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK); assert(r >= 0);
	r = mEngine->RegisterObjectBehaviour("Parameters", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ParametersConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = mEngine->RegisterObjectMethod("Parameters", "void setString(const string&in key, const string&in value) const", asMETHOD(Parameters, setString), asCALL_THISCALL); assert(r >= 0);
	r = mEngine->RegisterObjectMethod("Parameters", "void setBool(const string&in key, bool value) const", asMETHOD(Parameters, setBool), asCALL_THISCALL); assert(r >= 0);
	r = mEngine->RegisterObjectMethod("Parameters", "void setInt(const string&in key, int value) const", asMETHOD(Parameters, setInt), asCALL_THISCALL); assert(r >= 0);
	r = mEngine->RegisterObjectMethod("Parameters", "void setFloat(const string&in key, float value) const", asMETHOD(Parameters, setFloat), asCALL_THISCALL); assert(r >= 0);


	r = mEngine->RegisterObjectType("ContactInfo", sizeof(ContactInfo), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK); assert(r >= 0);
	r = mEngine->RegisterObjectBehaviour("ContactInfo", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ContactInfoConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = mEngine->RegisterObjectProperty("ContactInfo", "Vec2 point", asOFFSET(ContactInfo, point)); assert(r >= 0);
	r = mEngine->RegisterObjectProperty("ContactInfo", "Vec2 normal", asOFFSET(ContactInfo, normal)); assert(r >= 0);


	/*r = mEngine->RegisterObjectType("NetMessage", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
	//r = mEngine->RegisterObjectBehaviour("NetMessage", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(NetMessageConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = mEngine->RegisterObjectProperty("NetMessage", "uint8 mMsgId", asOFFSET(NetMessage, mMsgId)); assert(r >= 0);
	r = mEngine->RegisterObjectProperty("NetMessage", "uint16 mClientId", asOFFSET(NetMessage, mClientId)); assert(r >= 0);
	r = mEngine->RegisterObjectMethod("NetMessage", "void serializeBool(bool &inout value)", asMETHOD(NetMessage, serializeBool), asCALL_THISCALL); assert(r >= 0);
	r = mEngine->RegisterObjectMethod("NetMessage", "void serializeByte(int8 &inout value)", asMETHOD(NetMessage, serializeByte), asCALL_THISCALL); assert(r >= 0);
	r = mEngine->RegisterObjectMethod("NetMessage", "void serializeUByte(uint8 &inout value)", asMETHOD(NetMessage, serializeUByte), asCALL_THISCALL); assert(r >= 0);
	r = mEngine->RegisterObjectMethod("NetMessage", "void serializeShort(int16 &inout value)", asMETHOD(NetMessage, serializeShort), asCALL_THISCALL); assert(r >= 0);
	r = mEngine->RegisterObjectMethod("NetMessage", "void serializeUShort(uint16 &inout value)", asMETHOD(NetMessage, serializeUShort), asCALL_THISCALL); assert(r >= 0);
	r = mEngine->RegisterObjectMethod("NetMessage", "void serializeInt(int32 &inout value)", asMETHOD(NetMessage, serializeInt), asCALL_THISCALL); assert(r >= 0);
	r = mEngine->RegisterObjectMethod("NetMessage", "void serializeUInt(uint32 &inout value)", asMETHOD(NetMessage, serializeUInt), asCALL_THISCALL); assert(r >= 0);
	r = mEngine->RegisterObjectMethod("NetMessage", "void serializeFloat(float &inout value)", asMETHOD(NetMessage, serializeFloat), asCALL_THISCALL); assert(r >= 0);
	r = mEngine->RegisterObjectMethod("NetMessage", "void serializeString(string &inout value)", asMETHOD(NetMessage, serializeString), asCALL_THISCALL); assert(r >= 0);
	r = mEngine->RegisterObjectMethod("NetMessage", "bool isReading() const", asMETHOD(NetMessage, isReading), asCALL_THISCALL); assert(r >= 0);*/


	r = mEngine->RegisterObjectType("GameObject", 0, asOBJ_REF | asOBJ_NOCOUNT);
	//r = mEngine->RegisterObjectBehaviour("GameObject", asBEHAVE_ADDREF, "void f()", asMETHOD(ScriptObject, AddRef), asCALL_THISCALL); assert(r >= 0);
	//r = mEngine->RegisterObjectBehaviour("GameObject", asBEHAVE_RELEASE, "void f()", asMETHOD(ScriptObject, Release), asCALL_THISCALL); assert(r >= 0);
	r = mEngine->RegisterObjectMethod("GameObject", "uint32 getLocalIndex()", asMETHOD(ScriptObject, getLocalIndex), asCALL_THISCALL); assert(r >= 0);
	r = mEngine->RegisterObjectMethod("GameObject", "uint32 getType()", asMETHOD(ScriptObject, getType), asCALL_THISCALL); assert(r >= 0);
	r = mEngine->RegisterObjectMethod("GameObject", "Vec2 getPosition()", asMETHOD(ScriptObject, getPosition), asCALL_THISCALL); assert(r >= 0);

	r = mEngine->RegisterGlobalFunction("bool isServer()", asMETHOD(ScriptManager, isServer), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("bool isClient()", asMETHOD(ScriptManager, isClient), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);

	r = mEngine->RegisterGlobalFunction("void setGameOver()", asMETHOD(ScriptManager, setGameOver), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("bool isGameOver()", asMETHOD(ScriptManager, isGameOver), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);

	r = mEngine->RegisterGlobalFunction("void scoreUser(uint16 clientId)", asMETHOD(ScriptManager, scoreUser), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("void scoreTeam(int team)", asMETHOD(ScriptManager, scoreTeam), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);

	//r = mEngine->RegisterGlobalFunction("void spawnPlayer()", asMETHOD(ScriptManager, spawnPlayer), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("uint32 spawnObject(const string&in name, Vec2 pos, Parameters&in params)", asMETHOD(ScriptManager, spawnObject), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("void destroyObject(uint32 index)", asMETHOD(ScriptManager, destroyObject), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);

	r = mEngine->RegisterGlobalFunction("uint16 getClientIdFromPlayer(uint32 index)", asMETHOD(ScriptManager, getClientIdFromPlayer), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	
	r = mEngine->RegisterGlobalFunction("GameObject@ getGameObjectByName(const string&in name)", asMETHOD(ScriptManager, getGameObjectByName), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("uint32 getObjectIndexByName(const string&in name)", asMETHOD(ScriptManager, getObjectIndexByName), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);

	r = mEngine->RegisterGlobalFunction("array<GameObject@> @findObjectsByType(int type)", asMETHOD(ScriptManager, findObjectsByType), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("array<uint32> @findObjectsIndexByType(uint32 type)", asMETHOD(ScriptManager, findObjectsIndexByType), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	
	r = mEngine->RegisterGlobalFunction("uint32 getObjectType(uint32 index)", asMETHOD(ScriptManager, getObjectType), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("string getObjectName(uint32 index)", asMETHOD(ScriptManager, getObjectName), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("Vec2 getObjectPosition(uint32 index)", asMETHOD(ScriptManager, getObjectPosition), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("void setObjectPosition(uint32 index, const Vec2 &in pos)", asMETHOD(ScriptManager, setObjectPosition), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("float getObjectRotation(uint32 index)", asMETHOD(ScriptManager, getObjectRotation), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("void setObjectRotation(uint32 index, float rotation)", asMETHOD(ScriptManager, setObjectRotation), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("Vec2 getObjectVelocity(uint32 index)", asMETHOD(ScriptManager, getObjectRotation), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("void setObjectVelocity(uint32 index, const Vec2 &in vel)", asMETHOD(ScriptManager, setObjectVelocity), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);

	r = mEngine->RegisterGlobalFunction("void addObjectEvent(uint32 objIndex, const string &in eventName, const string &in funcName)", asMETHOD(ScriptManager, addObjectEvent), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	
	r = mEngine->RegisterGlobalFunction("void setBodyAwake(uint32 index, bool awake)", asMETHOD(ScriptManager, setBodyAwake), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);

	r = mEngine->RegisterGlobalFunction("void checkPlayerRespawn(uint32 respawnTime)", asMETHOD(ScriptManager, checkPlayerRespawn), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("int getNumPlayers()", asMETHOD(ScriptManager, getNumPlayers), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);

	r = mEngine->RegisterGlobalFunction("void setTeam(uint16 clientId, int team)", asMETHOD(ScriptManager, setTeam), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("int getTeam(uint16 clientId)", asMETHOD(ScriptManager, getTeam), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("uint32 getNumClientsInTeam(int team)", asMETHOD(ScriptManager, getNumClientsInTeam), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("array<uint16> @getClientsInTeam(int team)", asMETHOD(ScriptManager, getClientsInTeam), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);

	r = mEngine->RegisterGlobalFunction("void initScore(uint32 numTeams)", asMETHOD(ScriptManager, initScore), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("void setScore(uint32 index, int score)", asMETHOD(ScriptManager, setScore), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("void showGameTimer(bool show)", asMETHOD(ScriptManager, showGameTimer), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);
	r = mEngine->RegisterGlobalFunction("void setGameTimer(int time)", asMETHOD(ScriptManager, setGameTimer), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);

	r = mEngine->RegisterGlobalFunction("void enableInput(const string&in name, bool enable)", asMETHOD(ScriptManager, enableInput), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);

	r = mEngine->RegisterGlobalFunction("int randomInt(int min, int max)", asMETHOD(ScriptManager, randomInt), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);

	mGlobalScript = new Script(this, "global");
}

ScriptManager::~ScriptManager()
{
	for (size_t i = 0; i < mScriptObjects.size(); i++)
		delete mScriptObjects[i];
	mScriptObjects.clear();

	for (size_t i = 0; i < mContexts.size(); i++)
		mContexts[i]->Release();
	mContexts.clear();

	delete mGlobalScript;

	if (mEngine)
		mEngine->ShutDownAndRelease();
}

Script* ScriptManager::createScript(const std::string& name, const std::string& file)
{
	Script* script = new Script(this, name);
	script->createFromFile(file);
	return script;
}

bool ScriptManager::loadFile(const std::string& file)
{
	return mGlobalScript->createFromFile(file);
}

asIScriptContext* ScriptManager::prepareContextFromPool(asIScriptFunction* func)
{
	asIScriptContext *ctx = 0;
	if (mContexts.size())
	{
		ctx = *mContexts.rbegin();
		mContexts.pop_back();
	}
	else
		ctx = getEngine()->CreateContext();

	int r = ctx->Prepare(func); assert(r >= 0);

	return ctx;
}

void ScriptManager::returnContextToPool(asIScriptContext* ctx)
{
	if (!ctx)
		return;

	// Unprepare the context to free any objects that might be held
	// as we don't know when the context will be used again.
	int r = ctx->Unprepare();
	if (r == 0)
	{
		mContexts.push_back(ctx);
	}
	else
	{
		int a = 0;
	}
}

void ScriptManager::messageCallback(const asSMessageInfo* msg)
{
	std::string type = "ERR ";
	if (msg->type == asMSGTYPE_WARNING)
		type = "WARN";
	else if (msg->type == asMSGTYPE_INFORMATION)
		type == "INFO";

	if (msg->type == asMSGTYPE_ERROR)
	{
		//hasCompileErrors = true;
	}

	std::string str = std::string(msg->section) + " (" + cocos2d::StringUtils::toString(msg->row) + ", " + cocos2d::StringUtils::toString(msg->col) + ") " + std::string(msg->message);

	cocos2d::MessageBox(str.c_str(), "Error");
}

/*void ScriptManager::onContactEnter(GameObject* owner, GameObject* obj, b2Contact* contact)
{

}*/

ScriptObject* ScriptManager::createScriptObject(GameObject* obj)
{
	ScriptObject* scriptObj = new ScriptObject(this, obj);
	mScriptObjects.push_back(scriptObj);
	scriptObj->mLocalIndex = mScriptObjects.size() - 1;

	return scriptObj;
}

void ScriptManager::removeScriptObject(ScriptObject* obj)
{
	size_t index = obj->mLocalIndex;

	delete mScriptObjects[index];
	mScriptObjects[index] = mScriptObjects.back();
	mScriptObjects.pop_back();

	if (index < mScriptObjects.size())
		mScriptObjects[index]->mLocalIndex = index;
}

bool ScriptManager::isServer()
{
	return mGame->mNetwork->isServer();
}

bool ScriptManager::isClient()
{
	return mGame->mNetwork->isClient();
}

void ScriptManager::setGameOver()
{
	mGame->setGameOver();
}

bool ScriptManager::isGameOver()
{
	return mGame->isGameOver();
}

void ScriptManager::scoreUser(std::uint16_t clientId)
{
	mGame->scoreUser(clientId);
}

void ScriptManager::scoreTeam(int team)
{
	mGame->scoreTeam(team);
}

void ScriptManager::spawnPlayer(const std::string name)
{
	int a = 0;
}

std::uint32_t ScriptManager::spawnObject(const std::string& name, const cocos2d::Vec2 pos, Parameters& params)
{
	GameObject* obj = mGame->spawnGameObject(name, pos, params);
	if (obj)
		return obj->mLocalIndex;
	return std::numeric_limits<std::uint32_t>::max();
}

void ScriptManager::destroyObject(std::uint32_t index)
{
	GameObject* obj = mGame->getGameObject(index);
	if (obj)
	{
		obj->destroyMe();
	}
}

GameObject* ScriptManager::getGameObjectByName(const std::string& name)
{
	return mGame->getGameObjectByName(name);
}

std::uint32_t ScriptManager::getObjectIndexByName(const std::string& name)
{
	GameObject* obj = mGame->getGameObjectByName(name);
	if (obj)
	{
		return obj->getLocalIndex();
	}
	return std::numeric_limits<std::uint32_t>::max();
}

CScriptArray* ScriptManager::findObjectsByType(int type)
{
	asIScriptContext *ctx = asGetActiveContext();
	if (ctx)
	{
		asIScriptEngine* engine = ctx->GetEngine();
		asITypeInfo* t = engine->GetTypeInfoByDecl("array<GameObject@>");

		std::vector<ScriptObject*> objects;

		auto it = mGame->mGameObjects.begin();
		auto end = mGame->mGameObjects.end();
		while (it != end)
		{
			if ((*it)->mTypeId == type)
			{
				ScriptObject* obj = createScriptObject((*it));
				objects.push_back(obj);
			}
			++it;
		}

		CScriptArray* arr = CScriptArray::Create(t, objects.size());
		for (asUINT i = 0; i < arr->GetSize(); i++)
		{
			arr->SetValue(i, &objects[i]);
		}
		// The ref count for the returned handle was already set in the array's constructor
		return arr;
	}
	return 0;
}

class CScriptArray* ScriptManager::findObjectsIndexByType(std::uint32_t type)
{
	asIScriptContext *ctx = asGetActiveContext();
	if (ctx)
	{
		asIScriptEngine* engine = ctx->GetEngine();
		asITypeInfo* t = engine->GetTypeInfoByDecl("array<uint32>");

		std::vector<std::uint32_t> objects;

		auto it = mGame->mGameObjects.begin();
		auto end = mGame->mGameObjects.end();
		while (it != end)
		{
			if ((*it)->mTypeId == type)
			{
				objects.push_back((*it)->getLocalIndex());
			}
			++it;
		}

		CScriptArray* arr = CScriptArray::Create(t, objects.size());
		for (asUINT i = 0; i < arr->GetSize(); i++)
		{
			arr->SetValue(i, &objects[i]);
		}
		// The ref count for the returned handle was already set in the array's constructor
		return arr;
	}
	return 0;
}

std::uint16_t ScriptManager::getClientIdFromPlayer(std::uint32_t index)
{
	Player* player = dynamic_cast<Player*>(mGame->getGameObject(index));
	if (player)
	{
		return player->mClientId;
	}
	return NetworkManager::InvalidClientId;
}

std::uint32_t ScriptManager::getObjectType(std::uint32_t index)
{
	GameObject* obj = mGame->getGameObject(index);
	if (obj)
	{
		return obj->mTypeId;
	}
	return std::numeric_limits<std::uint32_t>::max();
}

std::string ScriptManager::getObjectName(std::uint32_t index)
{
	GameObject* obj = mGame->getGameObject(index);
	if (obj)
	{
		return obj->getName();
	}
	return "";
}

cocos2d::Vec2 ScriptManager::getObjectPosition(std::uint32_t index)
{
	cocos2d::Vec2 pos;

	GameObject* obj = mGame->getGameObject(index);
	if (obj)
	{
		pos = obj->getPosition();
	}
	return pos;
}

void ScriptManager::setObjectPosition(std::uint32_t index, const cocos2d::Vec2& pos)
{
	GameObject* obj = mGame->getGameObject(index);
	if (obj)
	{
		obj->setPosition(pos);
	}
}

float ScriptManager::getObjectRotation(std::uint32_t index)
{
	GameObject* obj = mGame->getGameObject(index);
	if (obj)
	{
		return obj->getRotation();
	}
	return 0.f;
}

void ScriptManager::setObjectRotation(std::uint32_t index, float rotation)
{
	GameObject* obj = mGame->getGameObject(index);
	if (obj)
	{
		obj->setRotation(rotation);
	}
}

cocos2d::Vec2 ScriptManager::getObjectVelocity(std::uint32_t index)
{
	GameObject* obj = mGame->getGameObject(index);
	if (obj && obj->mBody)
	{
		return Box2D2cocos2(obj->mBody->GetLinearVelocity());
	}
	return cocos2d::Vec2();
}

void ScriptManager::setObjectVelocity(std::uint32_t index, const cocos2d::Vec2& vel)
{
	GameObject* obj = mGame->getGameObject(index);
	if (obj && obj->mBody)
	{
		obj->mBody->SetLinearVelocity(cocos2d2Box2D(vel));
	}
}

void ScriptManager::setBodyAwake(std::uint32_t index, bool awake)
{
	GameObject* obj = mGame->getGameObject(index);
	if (obj && obj->mBody)
	{
		obj->mBody->SetAwake(awake);
	}
}

void ScriptManager::addObjectEvent(std::uint32_t objIndex, const std::string& eventName, const std::string& funcName)
{
	GameObject* obj = mGame->getGameObject(objIndex);
	if (obj)
	{
		ScriptFunctionEvent evt;
		evt.eventName = eventName;
		evt.funcName = funcName;
		obj->handleEvent(&evt);
	}
}

int ScriptManager::getNumPlayers()
{
	std::vector<Player*> players;
	mGame->findObjects(players);

	return players.size();
}

void ScriptManager::checkPlayerRespawn(std::uint32_t respawnTime)
{
	if (mGame->mGameMode)
	{
		mGame->mGameMode->checkPlayerRespawn(respawnTime);
	}
}

void ScriptManager::setTeam(std::uint16_t clientId, int team)
{
	if (mGame->mGameMode)
	{
		mGame->mGameMode->setTeam(clientId, team);
	}
}

int ScriptManager::getTeam(std::uint16_t clientId)
{
	if (mGame->mGameMode)
	{
		return mGame->mGameMode->getTeam(clientId);
	}
	return -1;
}

std::uint32_t ScriptManager::getNumClientsInTeam(int team)
{
	if (mGame->mGameMode)
		return mGame->mGameMode->getNumClientsInTeam(team);
	return 0;
}

class CScriptArray* ScriptManager::getClientsInTeam(int team)
{
	asIScriptContext *ctx = asGetActiveContext();
	if (ctx && mGame->mGameMode)
	{
		asIScriptEngine* engine = ctx->GetEngine();
		asITypeInfo* t = engine->GetTypeInfoByDecl("array<uint16>");

		std::vector<std::uint16_t> clients;
		mGame->mGameMode->getClientsInTeam(team, clients);

		CScriptArray* arr = CScriptArray::Create(t, clients.size());
		for (asUINT i = 0; i < arr->GetSize(); i++)
		{
			arr->SetValue(i, &clients[i]);
		}
		return arr;
	}
	return 0;
}

void ScriptManager::initScore(size_t numTeams)
{
	if (mGame->mGameMode)
	{
		mGame->mGameMode->initScore(numTeams);
	}
}

void ScriptManager::setScore(size_t index, int score)
{
	if (mGame->mGameMode)
	{
		mGame->mGameMode->setScore(index, score);
	}
}

void ScriptManager::showGameTimer(bool show)
{
	if (mGame->mGameMode)
	{
		mGame->mGameMode->showCountdown(show);
	}
}

void ScriptManager::setGameTimer(int time)
{
	if (mGame->mGameMode)
	{
		mGame->mGameMode->setCountdown(time);
	}
}

void ScriptManager::enableInput(const std::string& name, bool enable)
{
	if (enable)
	{
		mGame->mInput->activateAction(name);
	}
	else
	{
		mGame->mInput->deactivateAction(name);
	}
}

int ScriptManager::randomInt(int min, int max)
{
	return cocos2d::RandomHelper::random_int(min, max);
}