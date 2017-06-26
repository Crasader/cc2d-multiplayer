#include "NetMessage.h"

NetMessage::NetMessage()
	: mWOffset(0)
	, mROffset(0)
	, mMsgId(MSG_Null)
	, mIsReading(false)
{

}

NetMessage::NetMessage(std::uint8_t msgId)
	: mWOffset(0)
	, mROffset(0)
	, mMsgId(msgId)
	, mIsReading(false)
{
	writeUByte(msgId);
}

NetMessage::NetMessage(std::uint8_t msgId, std::uint16_t clientId)
	: mWOffset(0)
	, mROffset(0)
	, mMsgId(msgId)
	, mClientId(clientId)
	, mIsReading(false)
{
	writeUByte(msgId);
}

NetMessage::NetMessage(const char* data, const unsigned size)
	: mWOffset(0)
	, mROffset(0)
	, mIsReading(true)
{
	write(data, size);

	readUByte(mMsgId);
}

NetMessage::~NetMessage()
{
}

void NetMessage::clear()
{
	mBuffer.clear();
	mWOffset = 0;
	mROffset = 0;
}

bool NetMessage::isOnEnd() const
{
	return (mROffset >= mBuffer.size());
}

void NetMessage::write(const void* data, const unsigned size)
{
	if (data && size > 0)
	{
		std::size_t start = mBuffer.size();

		if (!mOnWriteData || mOnWriteData(data, size))
		{
			mBuffer.resize(start + size);
			memcpy(getData() + start, data, size);
			mWOffset = start + size;
		}
	}
}

void NetMessage::writeBool(const bool value)
{
	writeByte(value ? 1 : 0);
}

void NetMessage::writeByte(const std::int8_t value)
{
	write(&value, sizeof(std::int8_t));
}

void NetMessage::writeUByte(const std::uint8_t value)
{
	write(&value, sizeof(std::uint8_t));
}

void NetMessage::writeShort(const std::int16_t value)
{
	write(&value, sizeof(std::int16_t));
}

void NetMessage::writeUShort(const std::uint16_t value)
{
	write(&value, sizeof(std::uint16_t));
}

void NetMessage::writeInt(std::int32_t value)
{
	write(&value, sizeof(std::int32_t));
}

void NetMessage::writeUInt(std::uint32_t value)
{
	write(&value, sizeof(std::uint32_t));
}

void NetMessage::writeFloat(const float value)
{
	write(&value, sizeof(float));
}

void NetMessage::writeFloatShort(const std::float_t value, std::uint32_t cutOff)
{
	std::int16_t value16 = static_cast<std::int16_t>(std::round(value * (float)cutOff));
	writeShort(value16);
}

void NetMessage::writeFloatUShort(const std::float_t value, std::uint32_t cutOff)
{
	std::uint16_t value16 = static_cast<std::uint16_t>(std::round(value * (float)cutOff));
	writeUShort(value16);
}

void NetMessage::writeString(const std::string& value)
{
	std::uint16_t size = value.size();
	writeUShort(size);

	if (size > 0)
	{
		write(value.c_str(), size * sizeof(std::string::value_type));
	}
}

bool NetMessage::read(void* data, unsigned size)
{
	if (!mOnReadData || mOnReadData(data, size))
	{
		if (mROffset + size <= mBuffer.size())
		{
			memcpy(data, getData() + mROffset, size);
			mROffset += size;
			return true;
		}
	}
	return false;
}

bool NetMessage::readBool(bool& value)
{
	std::int8_t val;
	if (readByte(val))
	{
		value = (val == 0) ? false : true;
		return true;
	}
	return false;
}

bool NetMessage::readByte(std::int8_t& value)
{
	return read(&value, sizeof(std::int8_t));
}

bool NetMessage::readUByte(std::uint8_t& value)
{
	return read(&value, sizeof(std::uint8_t));
}

bool NetMessage::readShort(std::int16_t& value)
{
	return read(&value, sizeof(std::int16_t));
}

bool NetMessage::readUShort(std::uint16_t& value)
{
	return read(&value, sizeof(std::uint16_t));
}

bool NetMessage::readInt(std::int32_t& value)
{
	return read(&value, sizeof(std::int32_t));
}

bool NetMessage::readUInt(std::uint32_t& value)
{
	return read(&value, sizeof(std::uint32_t));
}

bool NetMessage::readFloat(float& value)
{
	return read(&value, sizeof(float));
}

bool NetMessage::readFloatShort(std::float_t& value, std::uint32_t cutOff)
{
	std::int16_t value16 = 0;
	if (readShort(value16))
	{
		value = value16 / (float)cutOff;
		return true;
	}
	return false;
}

bool NetMessage::readFloatUShort(std::float_t& value, std::uint32_t cutOff)
{
	std::uint16_t value16 = 0;
	if (readUShort(value16))
	{
		value = value16 / (float)cutOff;
		return true;
	}
	return false;
}

bool NetMessage::readString(std::string& value)
{
	value.clear();

	std::uint16_t size = 0;
	readUShort(size);

	if (size > 0 && mROffset + size <= mBuffer.size())
	{
		value.assign(&mBuffer[mROffset], size);
		mROffset += size;
		return true;
	}
	return false;
}

bool NetMessage::serializeBool(bool& value)
{
	if (isReading())
	{
		return readBool(value);
	}
	else
	{
		writeBool(value);
	}
	return true;
}

bool NetMessage::serializeByte(std::int8_t& value)
{
	if (isReading())
	{
		return readByte(value);
	}
	else
	{
		writeByte(value);
	}
	return true;
}

bool NetMessage::serializeUByte(std::uint8_t& value)
{
	if (isReading())
	{
		return readUByte(value);
	}
	else
	{
		writeUByte(value);
	}
	return true;
}

bool NetMessage::serializeShort(std::int16_t& value)
{
	if (isReading())
	{
		return readShort(value);
	}
	else
	{
		writeShort(value);
	}
	return true;
}

bool NetMessage::serializeUShort(std::uint16_t& value)
{
	if (isReading())
	{
		return readUShort(value);
	}
	else
	{
		writeUShort(value);
	}
	return true;
}

bool NetMessage::serializeInt(std::int32_t& value)
{
	if (isReading())
	{
		return readInt(value);
	}
	else
	{
		writeInt(value);
	}
	return true;
}

bool NetMessage::serializeUInt(std::uint32_t& value)
{
	if (isReading())
	{
		return readUInt(value);
	}
	else
	{
		writeUInt(value);
	}
	return true;
}

bool NetMessage::serializeFloat(std::float_t& value)
{
	if (isReading())
	{
		return readFloat(value);
	}
	else
	{
		writeFloat(value);
	}
	return true;
}

bool NetMessage::serializeFloat(std::float_t& value, std::uint32_t cutOff)
{
	if (isReading())
	{
		return readFloat(value);
	}
	else
	{
		value = (int)(std::round(value * (float)cutOff)) / (float)cutOff;
		writeFloat(value);
	}
	return true;
}

bool NetMessage::serializeFloatShort(std::float_t& value, std::uint32_t cutOff)
{
	if (isReading())
	{
		return readFloatShort(value, cutOff);
	}
	else
	{
		writeFloatShort(value, cutOff);
	}
	return true;
}

bool NetMessage::serializeFloatUShort(std::float_t& value, std::uint32_t cutOff)
{
	if (isReading())
	{
		return readFloatUShort(value, cutOff);
	}
	else
	{
		writeFloatUShort(value, cutOff);
	}
	return true;
}

bool NetMessage::serializeString(std::string& value)
{
	if (isReading())
	{
		return readString(value);
	}
	else
	{
		writeString(value);
	}
	return true;
}

bool NetMessage::isReading() const
{
	return mIsReading;
}