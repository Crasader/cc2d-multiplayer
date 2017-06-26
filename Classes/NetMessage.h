#ifndef __NETMESSAGE_H__
#define __NETMESSAGE_H__

#include <vector>
#include <cstdint>
#include <functional>

class NetMessage
{
public:
	enum eProtocol
	{
		MSG_Null,

		MSG_ConnectAccept,
		MSG_ConnectDenied,
		MSG_ServerInfo,
		MSG_ClientInfo,
		MSG_Connect,
		MSG_Disconnect,
		MSG_NameChanged,

		MSG_MapChange,
		MSG_UserScore,

		MSG_LobbyData,

		MSG_Lobby_UserData,
		MSG_Lobby_ChangeMap,
		MSG_Lobby_ExitGame,

		MSG_Voting,

		MSG_CreateObject,
		MSG_DestroyObject,
		MSG_Sync,
		MSG_Snap,
		MSG_AckSnap,
		MSG_ObjectEvent,
		MSG_ObjectEvent2,

		MSG_SpawnPlayer,
		MSG_Spectate,
		MSG_SpectateNext,

		MSG_ChatMsg,

		MSG_Num,
	};

public:
	NetMessage();
	NetMessage(std::uint8_t msgId);
	NetMessage(std::uint8_t msgId, std::uint16_t clientId);
	NetMessage(const char* data, const unsigned size);
	~NetMessage();

	void clear();

	bool isOnEnd() const;

	char* getData() { return &mBuffer[0]; };
	std::size_t getDataSize() const { return mBuffer.size(); }

	void write(const void* data, const unsigned size);
	void writeBool(const bool value);
	void writeByte(const std::int8_t value);
	void writeUByte(const std::uint8_t value);
	void writeShort(const std::int16_t value);
	void writeUShort(const std::uint16_t value);
	void writeInt(const std::int32_t value);
	void writeUInt(const std::uint32_t value);
	void writeFloat(const std::float_t value);
	void writeFloatShort(const std::float_t value, std::uint32_t cutOff);
	void writeFloatUShort(const std::float_t value, std::uint32_t cutOff);
	void writeString(const std::string& value);

	bool read(void* data, unsigned size);
	bool readBool(bool& value);
	bool readByte(std::int8_t& value);
	bool readUByte(std::uint8_t& value);
	bool readShort(std::int16_t& value);
	bool readUShort(std::uint16_t& value);
	bool readInt(std::int32_t& value);
	bool readUInt(std::uint32_t& value);
	bool readFloat(std::float_t& value);
	bool readFloatShort(std::float_t& value, std::uint32_t cutOff);
	bool readFloatUShort(std::float_t& value, std::uint32_t cutOff);
	bool readString(std::string& value);

	bool serializeBool(bool& value);
	bool serializeByte(std::int8_t& value);
	bool serializeUByte(std::uint8_t& value);
	bool serializeShort(std::int16_t& value);
	bool serializeUShort(std::uint16_t& value);
	bool serializeInt(std::int32_t& value);
	bool serializeUInt(std::uint32_t& value);
	bool serializeFloat(std::float_t& value);
	bool serializeFloat(std::float_t& value, std::uint32_t cutOff);
	bool serializeFloatShort(std::float_t& value, std::uint32_t cutOff);
	bool serializeFloatUShort(std::float_t& value, std::uint32_t cutOff);
	bool serializeString(std::string& value);

	bool isReading() const;

	std::vector<char> mBuffer;
	std::size_t mWOffset;
	std::size_t mROffset;

	std::uint8_t mMsgId;
	std::uint16_t mClientId;

	bool mIsReading;

	std::function<bool(const void* data, unsigned size)> mOnWriteData;
	std::function<bool(void* data, unsigned size)> mOnReadData;
};

#endif