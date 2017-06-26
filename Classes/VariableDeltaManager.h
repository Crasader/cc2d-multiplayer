#ifndef __VARIABLEDELTAMANAGER_H__
#define __VARIABLEDELTAMANAGER_H__

#include "NetMessage.h"

class VariableDeltaManager
{
public:
	VariableDeltaManager();
	~VariableDeltaManager();

	void setBasedMsg(NetMessage* basedMsg);

	size_t writeChanged(NetMessage* oldMsg, NetMessage& currMsg, NetMessage& deltaMsg);
	void writeDelta(NetMessage& oldMsg, NetMessage& currMsg, NetMessage& deltaMsg);

	void writeFromBase(NetMessage& msg, NetMessage* basedMsg, NetMessage& newMsg);

	bool onWriteData(const void* data, unsigned size);
	bool onReadData(void* data, unsigned size);

	void writeChangedVariables(NetMessage& msg);
	void readChangedVariables(NetMessage& msg);

private:
	bool variableChanged(std::uint32_t index);
	void setVariableChanged(std::uint32_t index, bool changed);

public:
	std::vector<std::uint8_t> mVariableChanged;
	std::uint32_t mCurrVariableIndex = 0;

	std::vector<std::uint8_t> mVariableSizes;

	NetMessage* mBasedMessage = nullptr;
};

#endif