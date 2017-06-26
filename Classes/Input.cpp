#include "Input.h"
#include "Parameters.h"

Input::Input()
{
	if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0)
	{
		throw std::exception();
	}

	mControllerHandles.resize(4);

	int numJoysticks = SDL_NumJoysticks();
	int ControllerIndex = 0;
	for (int joystickIndex = 0; joystickIndex < numJoysticks; ++joystickIndex)
	{
		if (!SDL_IsGameController(joystickIndex))
		{
			continue;
		}
		if (ControllerIndex >= 4)
		{
			break;
		}
		mControllerHandles[ControllerIndex] = SDL_GameControllerOpen(joystickIndex);
		ControllerIndex++;
	}
}

Input::~Input()
{
	for (size_t i = 0; i < mControllerHandles.size(); ++i)
	{
		if (mControllerHandles[i])
		{
			SDL_GameControllerClose(mControllerHandles[i]);
		}
	}
}

void Input::clearPendingActions()
{
	mActionFuncs.clear();
}

void Input::readConfig(Parameters& config)
{
	Parameters inputParam = config.getObject("Input");
}

InputAxis& Input::addAxis(const std::string& name, bool inverted, const std::string& group)
{
	InputAxis* axis = findAxis(name);
	if (!axis)
	{
		mAxisVec.emplace_back(name);
		axis = &mAxisVec.back();
	}

	axis->inverted = inverted;
	axis->group = group;

	return *axis;
}

InputAction& Input::addAction(const std::string& name, cocos2d::EventKeyboard::KeyCode key, const std::string& group)
{
	InputAction* action = findAction(name);
	if (!action)
	{
		mActionVec.emplace_back(name);
		action = &mActionVec.back();
	}

	action->group = group;

	action->addKey(key);

	return *action;
}

InputAction& Input::addAction(const std::string& name, SDL_GameControllerButton gamepadButton, const std::string& group)
{
	InputAction* action = findAction(name);
	if (!action)
	{
		mActionVec.emplace_back(name);
		action = &mActionVec.back();
	}

	action->group = group;

	action->addButton(gamepadButton);

	return *action;
}

InputAction& Input::addActionMouse(const std::string& name, int mouseButton, const std::string& group)
{
	InputAction* action = findAction(name);
	if (!action)
	{
		mActionVec.emplace_back(name);
		action = &mActionVec.back();
	}

	action->group = group;

	action->addMouseButton(mouseButton);

	return *action;
}

void Input::removeAction(const std::string& name)
{
	auto it = mAxisVec.begin();
	while (it != mAxisVec.end())
	{
		if (it->name == name)
			it = mAxisVec.erase(it);
		else
			++it;
	}
}

void Input::removeActionInGroup(const std::string& group)
{
	auto it = mAxisVec.begin();
	while (it != mAxisVec.end())
	{
		if (it->group == group)
			it = mAxisVec.erase(it);
		else
			++it;
	}
}

void Input::removeAllActions()
{
	mActionVec.clear();
}

void Input::activateAction(const std::string& name)
{
	InputAction* action = findAction(name);
	if (action)
	{
		action->isActive = true;
	}
}

void Input::deactivateAction(const std::string& name)
{
	InputAction* action = findAction(name);
	if (action)
	{
		action->isActive = false;
	}
}

void Input::activateAxis(const std::string& name)
{
	InputAxis* axis = findAxis(name);
	if (axis)
	{
		axis->isActive = true;
	}
}

void Input::deactivateAxis(const std::string& name)
{
	InputAxis* axis = findAxis(name);
	if (axis)
	{
		axis->isActive = false;
	}
}

void Input::activateAllInGroup(const std::string& group)
{
	auto acIt = mActionVec.begin();
	while (acIt != mActionVec.end())
	{
		if ((*acIt).group == group)
			(*acIt).isActive = true;
		++acIt;
	}
	auto axIt = mAxisVec.begin();
	while (axIt != mAxisVec.end())
	{
		if ((*axIt).group == group)
			(*axIt).isActive = true;
		++axIt;
	}
}

void Input::deactivateAllInGroup(const std::string& group)
{
	auto acIt = mActionVec.begin();
	while (acIt != mActionVec.end())
	{
		if ((*acIt).group == group)
			(*acIt).isActive = false;
		++acIt;
	}
	auto axIt = mAxisVec.begin();
	while (axIt != mAxisVec.end())
	{
		if ((*axIt).group == group)
			(*axIt).isActive = false;
		++axIt;
	}
}

InputAxis* Input::findAxis(const std::string& name)
{
	for (size_t i = 0; i < mAxisVec.size(); ++i)
	{
		if (mAxisVec[i].name == name)
		{
			return &mAxisVec[i];
		}
	}
	return 0;
}

InputAction* Input::findAction(const std::string& name)
{
	for (size_t i = 0; i < mActionVec.size(); ++i)
	{
		if (mActionVec[i].name == name)
		{
			return &mActionVec[i];
		}
	}
	return 0;
}

void Input::bindAxis(const std::string& name, const InputAxis::AxisFunc& axisFunc)
{
	InputAxis* axis = findAxis(name);
	if (axis)
	{
		axis->axisFunc = axisFunc;
	}

}

void Input::bindAction(const std::string& name, eInputEvent keyEvent, const ActionFunc& actionFunc, void* objPtr)
{
	InputAction* action = findAction(name);
	if (action)
	{
		action->addBind(keyEvent, actionFunc, objPtr);
	}
}

void Input::unbindAxis(const std::string& name)
{
	InputAxis* axis = findAxis(name);
	if (axis)
	{
		axis->axisFunc = nullptr;
	}
}

void Input::unbindAction(const std::string& name)
{
	InputAction* action = findAction(name);
	if (action)
	{
		action->bindings.clear();
	}
}

void Input::unbindAll()
{
	mAxisVec.clear();
	mActionVec.clear();
}

void Input::unbindAll(void* objPtr)
{
	auto acIt = mActionVec.begin();
	auto acEnd = mActionVec.end();
	while (acIt != acEnd)
	{
		auto bIt = (*acIt).bindings.begin();
		while (bIt != (*acIt).bindings.end())
		{
			if ((*bIt).mObjPtr == objPtr)
			{
				bIt = (*acIt).bindings.erase(bIt);
			}
			else
				++bIt;
		}
		++acIt;
	}
}

float Input::getAxisValue(const std::string& name)
{
	for (size_t i = 0; i < mAxisVec.size(); ++i)
	{
		if (mAxisVec[i].name == name)
		{
			if (!mAxisVec[i].isActive)
				return 0.f;

			return mAxisVec[i].value;
		}
	}
	return 0.f;
}

float Input::getGamepadAxisValue(int controllerIdx, int axis)
{
	SDL_GameController* gc = mControllerHandles[controllerIdx];
	Sint16 value = SDL_GameControllerGetAxis(gc, (SDL_GameControllerAxis)axis);

	if (axis >= SDL_CONTROLLER_AXIS_TRIGGERLEFT && axis <= SDL_CONTROLLER_AXIS_TRIGGERLEFT)
	{
		return value / 32767.f;
	}

	float norm = (value <= 0 ? 32768.f : 32767.f);
	return value / norm;
}

float Input::getGamepadButtonValue(int controllerIdx, int button)
{
	SDL_GameController* gc = mControllerHandles[controllerIdx];
	Uint8 value = SDL_GameControllerGetButton(gc, (SDL_GameControllerButton)button);
	return value ? 1.f : 0.f;
}

float Input::getKeyValue(cocos2d::EventKeyboard::KeyCode key)
{
	auto it = mKeyStateMap.find(key);
	if (it != mKeyStateMap.end())
	{
		return it->second.value;
	}
	return 0.f;
}

bool Input::isKeyPressed(cocos2d::EventKeyboard::KeyCode code) const
{
	auto it = mKeyStateMap.find(code);
	if (it != mKeyStateMap.end())
	{
		return it->second.isPressed;
	}
	return false;
}

bool Input::isKeyReleased(cocos2d::EventKeyboard::KeyCode code) const
{
	return !isKeyPressed(code);
}

void Input::onKeyPressed(cocos2d::EventKeyboard::KeyCode code, cocos2d::Event* event)
{
	mIsGamePadActive = false;

	auto it = mKeyStateMap.find(code);
	if (it == mKeyStateMap.end())
	{
		auto p = mKeyStateMap.insert(std::make_pair(code, KeyState(code)));
		it = p.first;
	}

	it->second.isPressed = true;
	it->second.value = 1.0f;

	auto aIt = mActionVec.begin();
	auto aEnd = mActionVec.end();
	while (aIt != aEnd)
	{
		if ((*aIt).isActive)
		{
			auto keyIt = aIt->keyMap.find(code);
			if (keyIt != aIt->keyMap.end())
			{
				auto biIt = aIt->bindings.begin();
				auto biEnd = aIt->bindings.end();
				while (biIt != biEnd)
				{
					if (biIt->keyEvent == IE_Pressed)
					{
						mActionFuncs.push_back(biIt->actionFunc);
					}
					++biIt;
				}
			}
		}
		++aIt;
	}
}

void Input::onKeyReleased(cocos2d::EventKeyboard::KeyCode code, cocos2d::Event* event)
{
	mIsGamePadActive = false;

	auto it = mKeyStateMap.find(code);
	if (it == mKeyStateMap.end())
	{
		auto p = mKeyStateMap.insert(std::make_pair(code, KeyState(code)));
		it = p.first;
	}

	it->second.isPressed = false;
	it->second.value = 0.0f;

	auto aIt = mActionVec.begin();
	auto aEnd = mActionVec.end();
	while (aIt != aEnd)
	{
		if ((*aIt).isActive)
		{
			auto keyIt = aIt->keyMap.find(code);
			if (keyIt != aIt->keyMap.end())
			{
				auto biIt = aIt->bindings.begin();
				auto biEnd = aIt->bindings.end();
				while (biIt != biEnd)
				{
					if (biIt->keyEvent == IE_Released)
					{
						mActionFuncs.push_back(biIt->actionFunc);
					}
					++biIt;
				}
			}
		}
		++aIt;
	}
}

void Input::onGamepadButton(int index, int button, bool pressed)
{
	mIsGamePadActive = true;

	auto aIt = mActionVec.begin();
	auto aEnd = mActionVec.end();
	while (aIt != aEnd)
	{
		if ((*aIt).isActive)
		{
			auto buttonIt = aIt->gamepadMap.find(button);
			if (buttonIt != aIt->gamepadMap.end())
			{
				auto biIt = aIt->bindings.begin();
				auto biEnd = aIt->bindings.end();
				while (biIt != biEnd)
				{
					if ((biIt->keyEvent == IE_Pressed && pressed) ||
						(biIt->keyEvent == IE_Released && !pressed))
					{
						mActionFuncs.push_back(biIt->actionFunc);
					}
					++biIt;
				}
			}
		}
		++aIt;
	}
}

void Input::onGamepadAxis(int index, int axis, float value)
{
	mIsGamePadActive = true;
}

void Input::onMouseMove(cocos2d::Event *event)
{
	mIsGamePadActive = false;

	cocos2d::EventMouse* e = (cocos2d::EventMouse*)event;
	mCursorPosX = e->getCursorX();
	mCursorPosY = e->getCursorY();
	//str = str + tostr(e->getCursorX()) + " Y:" + tostr(e->getCursorY());
}

void Input::onMouseDown(cocos2d::Event *event)
{
	mIsGamePadActive = false;

	cocos2d::EventMouse* e = (cocos2d::EventMouse*)event;

	int button = e->getMouseButton();

	auto aIt = mActionVec.begin();
	auto aEnd = mActionVec.end();
	while (aIt != aEnd)
	{
		if ((*aIt).isActive)
		{
			auto buttonIt = aIt->mouseMap.find(button);
			if (buttonIt != aIt->mouseMap.end())
			{
				auto biIt = aIt->bindings.begin();
				auto biEnd = aIt->bindings.end();
				while (biIt != biEnd)
				{
					if (biIt->keyEvent == IE_Pressed)
					{
						mActionFuncs.push_back(biIt->actionFunc);
					}
					++biIt;
				}
			}
		}
		++aIt;
	}
}

void Input::onMouseUp(cocos2d::Event *event)
{
	mIsGamePadActive = false;

	cocos2d::EventMouse* e = (cocos2d::EventMouse*)event;

	int button = e->getMouseButton();

	auto aIt = mActionVec.begin();
	auto aEnd = mActionVec.end();
	while (aIt != aEnd)
	{
		if ((*aIt).isActive)
		{
			auto buttonIt = aIt->mouseMap.find(button);
			if (buttonIt != aIt->mouseMap.end())
			{
				auto biIt = aIt->bindings.begin();
				auto biEnd = aIt->bindings.end();
				while (biIt != biEnd)
				{
					if (biIt->keyEvent == IE_Released)
					{
						mActionFuncs.push_back(biIt->actionFunc);
					}
					++biIt;
				}
			}
		}
		++aIt;
	}
}

void Input::onMouseScroll(cocos2d::Event *event)
{
	mIsGamePadActive = false;

	cocos2d::EventMouse* e = (cocos2d::EventMouse*)event;
	//str = str + tostr(e->getScrollX()) + " Y: " + tostr(e->getScrollY());
}

void Input::processInput(float deltaTime)
{
	SDL_Event evt;
	while (SDL_PollEvent(&evt))
	{
		switch (evt.type)
		{
		case SDL_CONTROLLERDEVICEADDED:
		{
			int id = evt.cdevice.which;
			if (SDL_IsGameController(id)) {
				SDL_GameController *pad = SDL_GameControllerOpen(id);
				if (pad) {
					SDL_Joystick *joy = SDL_GameControllerGetJoystick(pad);
					int instanceID = SDL_JoystickInstanceID(joy);
					
					mControllerHandles[id] = pad;
				}
			}
			break;
		}
		case SDL_CONTROLLERDEVICEREMOVED:
		{
			int id = evt.cdevice.which;

			SDL_GameController *pad = mControllerHandles[id];
			SDL_GameControllerClose(pad);
			mControllerHandles[id] = nullptr;
		}
		case SDL_CONTROLLERAXISMOTION:
		{
			int idx = evt.caxis.which;
			onGamepadAxis(idx, evt.caxis.axis, evt.caxis.value);

			break;
		}
		case SDL_CONTROLLERBUTTONDOWN:
		{
			int idx = evt.cbutton.which;
			onGamepadButton(idx, evt.cbutton.button, true);

			break;
		}
		case SDL_CONTROLLERBUTTONUP:
		{
			int idx = evt.cbutton.which;
			onGamepadButton(idx, evt.cbutton.button, false);

			break;
		}
		}
	}

	for (size_t i = 0; i < mActionFuncs.size(); ++i)
	{
		mActionFuncs[i]();
	}
	mActionFuncs.clear();

	auto aIt = mAxisVec.begin();
	auto aEnd = mAxisVec.end();
	while (aIt != aEnd)
	{
		if ((*aIt).isActive)
		{
			float value = 0.f;

			for (auto ap : (*aIt).props)
			{
				if (ap.isGamePad)
				{
					if (ap.gamepadAxis != SDL_CONTROLLER_AXIS_INVALID)
						value += getGamepadAxisValue(0, ap.gamepadAxis) * ap.scale;
					else
						value += getGamepadButtonValue(0, ap.gamepadButton) * ap.scale;
				}
				else
					value += getKeyValue(ap.key) * ap.scale;
			}

			value = std::max(std::min(value, 1.0f), -1.0f);

			if ((*aIt).inverted)
			{
				value *= -1.0f;
			}
			(*aIt).value = value;

			if ((*aIt).axisFunc != nullptr)
				(*aIt).axisFunc(value);
		}
		++aIt;
	}
}

void Input::loadSettings(const std::string& filename)
{

}