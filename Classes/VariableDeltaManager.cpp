#include "VariableDeltaManager.h"

VariableDeltaManager::VariableDeltaManager()
{
}

VariableDeltaManager::~VariableDeltaManager()
{
	if (mBasedMessage)
	{
		mBasedMessage->mOnWriteData = nullptr;
		mBasedMessage->mOnReadData = nullptr;
	}
}

void VariableDeltaManager::setBasedMsg(NetMessage* basedMsg)
{
	if (basedMsg)
	{
		if (basedMsg->isReading())
		{
			basedMsg->mOnReadData = std::bind(&VariableDeltaManager::onReadData, this, std::placeholders::_1, std::placeholders::_2);
		}
	}
	else
	{
		mBasedMessage->mOnWriteData = nullptr;
		mBasedMessage->mOnReadData = nullptr;
	}

	mBasedMessage = basedMsg;
}

size_t VariableDeltaManager::writeChanged(NetMessage* oldMsg, NetMessage& currMsg, NetMessage& deltaMsg)
{
	if (mVariableSizes.size() == 0)
		return 0;

	mVariableChanged.clear();
	mCurrVariableIndex = 0;

	size_t dataSize = 0;

	size_t offset = currMsg.mROffset;

	for (size_t i = 0; i < mVariableSizes.size(); ++i)
	{
		size_t size = mVariableSizes[i];
		void* data = currMsg.getData() + offset;

		if (oldMsg)
		{
			if (memcmp(oldMsg->getData() + offset, data, size) == 0)
			{
				setVariableChanged(i, false);
			}
			else
			{
				setVariableChanged(i, true);
				dataSize += size;
			}
		}
		else
		{
			setVariableChanged(i, true);
		}

		offset += size;
	}

	return dataSize;
}

void VariableDeltaManager::writeDelta(NetMessage& oldMsg, NetMessage& currMsg, NetMessage& deltaMsg)
{
	if (mVariableSizes.size() == 0)
		return;

	size_t offset = currMsg.mROffset;

	for (size_t i = 0; i < mVariableSizes.size(); ++i)
	{
		size_t size = mVariableSizes[i];
		void* data = currMsg.getData() + offset;

		if (!(memcmp(oldMsg.getData() + offset, data, size) == 0))
		{
			deltaMsg.write(data, size);
		}

		offset += size;
	}
}

void VariableDeltaManager::writeFromBase(NetMessage& msg, NetMessage* basedMsg, NetMessage& newMsg)
{
	if (mVariableSizes.size() == 0)
		return;

	while (!basedMsg->isOnEnd())
	{
		std::uint8_t byte = 0;
		basedMsg->readUByte(byte);
		if (((byte >> 0) & 0x1) == 0)
		{
			break;
		}
	}

	std::uint32_t start = sizeof(std::uint32_t) + sizeof(std::uint8_t) * mVariableChanged.size();

	std::uint32_t bOffset = start;
	std::uint32_t dOffset = start;
	for (size_t i = 0; i < mVariableSizes.size(); ++i)
	{
		std::uint32_t d = 0;
		std::uint32_t size = mVariableSizes[i];
		if (variableChanged(i))
		{
			newMsg.write(msg.getData() + dOffset, size);
			memcpy(&d, msg.getData() + dOffset, sizeof(std::uint32_t));
		}
		else if (bOffset <= basedMsg->mWOffset)
		{
			newMsg.write(basedMsg->getData() + bOffset, size);
			memcpy(&d, basedMsg->getData() + bOffset, sizeof(std::uint32_t));
		}
		else
		{
			int a = 0;
		}

		dOffset += size;
		bOffset += size;
	}
}

bool VariableDeltaManager::onWriteData(const void* data, unsigned size)
{
	mVariableSizes.push_back(size);

	return true;
}

bool VariableDeltaManager::onReadData(void* data, unsigned size)
{
	return variableChanged(mCurrVariableIndex++);
}

void VariableDeltaManager::writeChangedVariables(NetMessage& msg)
{
	for (size_t j = 0; j < mVariableChanged.size(); ++j)
	{
		msg.writeUByte(mVariableChanged[j]);
	}
}

void VariableDeltaManager::readChangedVariables(NetMessage& msg)
{
	while (!msg.isOnEnd())
	{
		std::uint8_t byte = 0;
		msg.readUByte(byte);

		mVariableChanged.push_back(byte);
		if (((byte >> 0) & 0x1) == 0)
		{
			return;
		}
	}
}

bool VariableDeltaManager::variableChanged(std::uint32_t index)
{
	std::uint32_t i = index / 7;
	std::uint32_t t = (index % 7) + 1;

	if (i >= mVariableChanged.size())
		return true;

	return ((mVariableChanged[i] >> t) & 0x1);
}

void VariableDeltaManager::setVariableChanged(std::uint32_t index, bool changed)
{ 
	std::uint32_t i = index / 7;
	std::uint32_t t = (index % 7) + 1;

	if (i >= mVariableChanged.size())
	{
		if (i > 0)
		{
			mVariableChanged[i - 1] |= (1 << 0);
		}

		mVariableChanged.emplace_back(0);
	}

	if (changed)
		mVariableChanged[i] |= (1 << t);
	else
	{
		mVariableChanged[i] &= ~(1 << t);
	}
}