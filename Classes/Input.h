#ifndef __INPUT_H__
#define __INPUT_H__

#include "cocos2d.h"
#include <SDL.h>

class Parameters;

enum eInputEvent
{
	IE_Pressed,
	IE_Released,
};

struct KeyState
{
	KeyState(const cocos2d::EventKeyboard::KeyCode& _code)
		: code(_code)
	{}

	cocos2d::EventKeyboard::KeyCode code;
	bool isPressed = false;
	float value = 0.f;
};

struct InputAxisProp
{
	InputAxisProp()
	{}

	InputAxisProp(cocos2d::EventKeyboard::KeyCode _key, float _scale)
		: key(_key)
		, gamepadAxis(SDL_CONTROLLER_AXIS_INVALID)
		, gamepadButton(SDL_CONTROLLER_BUTTON_INVALID)
		, scale(_scale)
		, isGamePad(false)
	{}
	InputAxisProp(SDL_GameControllerButton button, float _scale)
		: key(cocos2d::EventKeyboard::KeyCode::KEY_NONE)
		, gamepadAxis(SDL_CONTROLLER_AXIS_INVALID)
		, gamepadButton(button)
		, scale(_scale)
		, isGamePad(true)
	{}
	InputAxisProp(SDL_GameControllerAxis axis, float _scale)
		: key(cocos2d::EventKeyboard::KeyCode::KEY_NONE)
		, gamepadAxis(axis)
		, gamepadButton(SDL_CONTROLLER_BUTTON_INVALID)
		, scale(_scale)
		, isGamePad(true)
	{}

	cocos2d::EventKeyboard::KeyCode key;
	SDL_GameControllerButton gamepadButton;
	SDL_GameControllerAxis gamepadAxis;
	
	float scale = 1.f;

	bool isGamePad;
};

struct InputAxis
{
	typedef std::function<void(float)> AxisFunc;

	InputAxis()
	{}

	InputAxis(const std::string& _name)
		: name(_name)
	{}

	void clear()
	{
		props.clear();
		axisFunc = nullptr;
	}

	InputAxis& addProp(cocos2d::EventKeyboard::KeyCode key, float scale)
	{
		props.emplace_back(key, scale);
		return *this;
	}
	InputAxis& addProp(SDL_GameControllerButton button, float scale)
	{
		props.emplace_back(button, scale);
		return *this;
	}
	InputAxis& addProp(SDL_GameControllerAxis axis, float scale)
	{
		props.emplace_back(axis, scale);
		return *this;
	}

	std::string name;
	std::string group;
	bool inverted = false;
	float value = 0.f;
	bool isActive = true;

	std::vector<InputAxisProp> props;
	AxisFunc axisFunc;
};

struct InputActionBinding
{
	typedef std::function<void()> ActionFunc;

	InputActionBinding()
		: mObjPtr(0)
	{}
	InputActionBinding(eInputEvent _keyEvent, const ActionFunc& _actionFunc, void* objPtr)
		: keyEvent(_keyEvent)
		, actionFunc(_actionFunc)
		, mObjPtr(objPtr)
	{}

	eInputEvent keyEvent = eInputEvent::IE_Pressed;
	ActionFunc actionFunc;
	void* mObjPtr = nullptr;
};

struct InputAction
{
	InputAction()
	{}
	InputAction(const std::string& _name)
		: name(_name)
	{}

	InputAction& addKey(cocos2d::EventKeyboard::KeyCode keyCode)
	{
		keyMap.insert(keyCode);
		return *this;
	}

	InputAction& addButton(SDL_GameControllerButton button)
	{
		gamepadMap.insert(button);
		return *this;
	}

	InputAction& addMouseButton(int mouseButton)
	{
		mouseMap.insert(mouseButton);
		return *this;
	}

	InputAction& addBind(eInputEvent keyEvent, const InputActionBinding::ActionFunc& actionFunc, void* objPtr)
	{
		bindings.emplace_back(keyEvent, actionFunc, objPtr);
		return *this;
	}

	std::string name;
	std::string group;
	std::set<cocos2d::EventKeyboard::KeyCode> keyMap;
	std::set<int> gamepadMap;
	std::set<int> mouseMap;
	std::vector<InputActionBinding> bindings;
	bool isActive = true;
};

class Input
{
public:
	typedef InputActionBinding::ActionFunc ActionFunc;

public:
	Input();
	~Input();

	void clearPendingActions();

	void readConfig(Parameters& config);

	InputAxis& addAxis(const std::string& name, bool inverted = false, const std::string& group = "default");
	InputAction& addAction(const std::string& name, cocos2d::EventKeyboard::KeyCode key, const std::string& group = "default");
	InputAction& addAction(const std::string& name, SDL_GameControllerButton gamepadButton, const std::string& group = "default");
	InputAction& addActionMouse(const std::string& name, int mouseButton, const std::string& group = "default");

	void removeAction(const std::string& name);
	void removeActionInGroup(const std::string& group);
	void removeAllActions();

	void activateAction(const std::string& name);
	void deactivateAction(const std::string& name);

	void activateAxis(const std::string& name);
	void deactivateAxis(const std::string& name);

	void activateAllInGroup(const std::string& group);
	void deactivateAllInGroup(const std::string& group);

	InputAxis* findAxis(const std::string& name);
	InputAction* findAction(const std::string& name);

	void bindAxis(const std::string& name, const InputAxis::AxisFunc& axisFunc);
	void bindAction(const std::string& name, eInputEvent keyEvent, const ActionFunc& actionFunc, void* objPtr);

	void unbindAxis(const std::string& name);
	void unbindAction(const std::string& name);

	void unbindAll();
	void unbindAll(void* objPtr);

	float getAxisValue(const std::string& name);
	float getGamepadAxisValue(int controllerIdx, int axis);
	float getGamepadButtonValue(int controllerIdx, int button);
	float getKeyValue(cocos2d::EventKeyboard::KeyCode key);

	bool isKeyPressed(cocos2d::EventKeyboard::KeyCode code) const;
	bool isKeyReleased(cocos2d::EventKeyboard::KeyCode code) const;

	void onKeyPressed(cocos2d::EventKeyboard::KeyCode code, cocos2d::Event* event);
	void onKeyReleased(cocos2d::EventKeyboard::KeyCode code, cocos2d::Event* event);

	void onGamepadButton(int index, int button, bool pressed);
	void onGamepadAxis(int index, int axis, float value);

	void onMouseMove(cocos2d::Event *event);
	void onMouseDown(cocos2d::Event *event);
	void onMouseUp(cocos2d::Event *event);
	void onMouseScroll(cocos2d::Event *event);

	void processInput(float deltaTime);

	void loadSettings(const std::string& filename);

private:
	std::unordered_map<cocos2d::EventKeyboard::KeyCode, KeyState> mKeyStateMap;

	std::vector<InputAxis> mAxisVec;
	std::vector<InputAction> mActionVec;
	std::vector<ActionFunc> mActionFuncs;

	float mCursorPosX = 0.f;
	float mCursorPosY = 0.f;

	float mVibrateValue = 0.0f;
	float mVibrateTimer = 0.0f;

	bool mIsGamePadActive = false;

	std::vector<SDL_GameController*> mControllerHandles;
};

#endif