#include "Voting.h"

#include "NetMessage.h"

#include "GameSystem.h"
#include "NetworkManager.h"
#include "Lobby.h"
#include "Input.h"

Voting::Voting(GameSystem* game)
	: mGame(game)
{
	mTextLabel = cocos2d::Label::createWithTTF("", "fonts/Marker Felt.ttf", 20.0f);
	mTextLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_BOTTOM_LEFT);
	mTextLabel->setColor(cocos2d::Color3B::GREEN);
	mTextLabel->setPosition(10.0f, 600.0f);
	mTextLabel->setVisible(false);

	mGame->mInput->bindAction("voteCancel", IE_Pressed, std::bind(&Voting::onCancel, this), this);
	mGame->mInput->bindAction("voteOption1", IE_Pressed, std::bind(&Voting::onOption1, this), this);
	mGame->mInput->bindAction("voteOption2", IE_Pressed, std::bind(&Voting::onOption2, this), this);
	mGame->mInput->bindAction("voteOption3", IE_Pressed, std::bind(&Voting::onOption3, this), this);
	mGame->mInput->bindAction("voteOption4", IE_Pressed, std::bind(&Voting::onOption4, this), this);
	mGame->mInput->bindAction("voteOption5", IE_Pressed, std::bind(&Voting::onOption5, this), this);
	mGame->mInput->bindAction("voteOption6", IE_Pressed, std::bind(&Voting::onOption6, this), this);
	mGame->mInput->bindAction("voteOption7", IE_Pressed, std::bind(&Voting::onOption7, this), this);
	mGame->mInput->bindAction("voteOption8", IE_Pressed, std::bind(&Voting::onOption8, this), this);
	mGame->mInput->bindAction("voteOption9", IE_Pressed, std::bind(&Voting::onOption9, this), this);
}

Voting::~Voting()
{

}

void Voting::reset()
{
	mCloseTime = 0.f;
	mIsSelecting = false;
	mHasVoted = false;
	mClientsHasVoted.clear();
	mVoteType = VT_None;
	mVoteOptions.clear();
	mVoteText.clear();
	mVoteName.clear();
	mNumTotalVotes = 0;

	mTextLabel->setVisible(false);
}

void Voting::startVote()
{
	if (!mGame->mNetwork->isServer())
		return;

	mNumTotalVotes = 0;
	for (size_t i = 0; i < mGame->mLobby->mLobbyUsers.size(); ++i)
	{
		if (mGame->mLobby->mLobbyUsers[i].isConnected)
		{
			mNumTotalVotes++;
		}
	}

	mVoteOptions.clear();
	{
		VoteOption option;
		option.name = "Kick Player";
		mVoteOptions.push_back(option);
	}

	{
		VoteOption option;
		option.name = "Change Map";
		mVoteOptions.push_back(option);
	}

	mVoteText = "";
	updateVoteOptionText(true);
	mTextLabel->setVisible(true);

	mCloseTime = 30.f;
	mIsSelecting = true;
	mHasVoted = false;
	mClientsHasVoted.clear();
	mVoteType = VT_None;
}

bool Voting::isVisible() const
{
	return mTextLabel->isVisible();
}

void Voting::vote(VoteType type, std::uint8_t option)
{
	
}

void Voting::onCancel()
{
	if (mGame->mNetwork->isServer())
	{
		if (mVoteType != VT_None)
		{
			NetMessage abortMsg(NetMessage::MSG_Voting);
			abortMsg.writeUByte(mVoteType);
			abortMsg.writeUByte(VO_Aborted);
			mGame->mNetwork->send(abortMsg);
		}

		mTextLabel->setVisible(false);
		mCloseTime = 0.f;
		mVoteType = VT_None;
	}
}

void Voting::onOption(std::uint8_t option)
{
	if (!isVisible() || mHasVoted)
		return;

	if (option >= mVoteOptions.size())
		return;

	if (mVoteType == VT_None)
	{
		mVoteType = (VoteType)(option + 1);

		mCloseTime = 120.f;

		mVoteOptions.clear();

		if (mVoteType == VT_KickPlayer)
		{
			mNumTotalVotes--;
			for (size_t i = 0; i < mGame->mLobby->mLobbyUsers.size(); ++i)
			{
				if (mGame->mLobby->mLobbyUsers[i].isConnected && mGame->mLobby->mLobbyUsers[i].clientId != NetworkManager::ServerId)
				{
					VoteOption option;
					option.name = mGame->mLobby->mLobbyUsers[i].name;
					mVoteOptions.push_back(option);
				}
			}

			mVoteText = "Kick Player:";
			updateVoteOptionText(true);
		}
		else if (mVoteType == VT_MapChange)
		{
			VoteOption option;
			option.name = "Random";
			mVoteOptions.push_back(option);

			mVoteText = "Change Map:";
			updateVoteOptionText(true);
		}
	}
	else if (mVoteType == VT_KickPlayer)
	{
		if (mIsSelecting)
		{
			mIsSelecting = false;

			const VoteOption voteOption = mVoteOptions[(int)option];
			std::string playerName = voteOption.name;
			mVoteName = playerName;

			mVoteOptions.clear();
			{
				VoteOption yesOption;
				yesOption.name = "Yes";
				yesOption.numVotes = 1;
				mVoteOptions.push_back(yesOption);
			}
			{
				VoteOption noOption;
				noOption.name = "No";
				mVoteOptions.push_back(noOption);
			}

			mHasVoted = true;
			mVoteText = "Kick Player: " + playerName;
			updateVoteOptionText(mGame->mNetwork->isServer());

			NetMessage voteMsg(NetMessage::MSG_Voting);
			voteMsg.writeUByte(VT_KickPlayer);
			voteMsg.writeUByte(VO_Start);
			voteMsg.writeString(playerName);
			mGame->mNetwork->send(voteMsg);
		}
		else
		{
			mHasVoted = true;

			NetMessage voteMsg(NetMessage::MSG_Voting);
			voteMsg.writeUByte(VT_KickPlayer);
			voteMsg.writeUByte(VO_Vote);
			voteMsg.writeUByte(option);
			mGame->mNetwork->send(voteMsg);
		}
	}
	else if (mVoteType == VT_MapChange)
	{
		if (mIsSelecting)
		{
			mIsSelecting = false;

			const VoteOption voteOption = mVoteOptions[(int)option];
			std::string map = voteOption.name;
			mVoteName = map;

			mVoteOptions.clear();
			{
				VoteOption yesOption;
				yesOption.name = "Yes";
				yesOption.numVotes = 1;
				mVoteOptions.push_back(yesOption);
			}
			{
				VoteOption noOption;
				noOption.name = "No";
				mVoteOptions.push_back(noOption);
			}

			mHasVoted = true;

			mVoteText = "Change Map: " + map;
			updateVoteOptionText(mGame->mNetwork->isServer());

			NetMessage voteMsg(NetMessage::MSG_Voting);
			voteMsg.writeUByte(VT_MapChange);
			voteMsg.writeUByte(VO_Start);
			voteMsg.writeString(map);
			mGame->mNetwork->send(voteMsg);
		}
		else
		{
			mHasVoted = true;

			NetMessage voteMsg(NetMessage::MSG_Voting);
			voteMsg.writeUByte(VT_MapChange);
			voteMsg.writeUByte(VO_Vote);
			voteMsg.writeUByte(option);
			mGame->mNetwork->send(voteMsg);
		}
	}
}

void Voting::onOption1()
{
	onOption(0);
}

void Voting::onOption2()
{
	onOption(1);
}

void Voting::onOption3()
{
	onOption(2);
}

void Voting::onOption4()
{
	onOption(3);
}

void Voting::onOption5()
{
	onOption(4);
}

void Voting::onOption6()
{
	onOption(5);
}

void Voting::onOption7()
{
	onOption(6);
}

void Voting::onOption8()
{
	onOption(7);
}

void Voting::onOption9()
{
	onOption(8);
}

void Voting::onVoteOver()
{
	if (mVoteType != VT_None && mHasVoted)
	{
		int totalVotes = 0;
		for (size_t i = 0; i < mVoteOptions.size(); ++i)
		{
			totalVotes += mVoteOptions[i].numVotes;
		}
		if (totalVotes >= mNumTotalVotes)
		{
			if (mVoteOptions[0].numVotes > mVoteOptions[1].numVotes)
			{
				NetMessage acceptMsg(NetMessage::MSG_Voting);
				acceptMsg.writeUByte(mVoteType);
				acceptMsg.writeUByte(VO_Accepted);
				mGame->mNetwork->send(acceptMsg);

				if (mVoteType == VT_KickPlayer)
				{
					for (size_t i = 0; i < mGame->mLobby->mLobbyUsers.size(); ++i)
					{
						if (mGame->mLobby->mLobbyUsers[i].isConnected && mGame->mLobby->mLobbyUsers[i].name == mVoteName)
						{
							mGame->mNetwork->kickClient(mGame->mLobby->mLobbyUsers[i].clientId);
						}
					}
				}
				else if (mVoteType == VT_MapChange)
				{
					if (mVoteName == "Random")
					{
						mGame->changeToNextRandomMap();
					}
					else
					{
						mGame->setNextMapName(mVoteName);
						mGame->wantsRestart();
					}
				}

				mCloseTime = 0.f;
				mTextLabel->setVisible(false);
				mVoteType = VT_None;
			}
			else
			{
				NetMessage failedMsg(NetMessage::MSG_Voting);
				failedMsg.writeUByte(mVoteType);
				failedMsg.writeUByte(VO_Failed);
				mGame->mNetwork->send(failedMsg);

				mCloseTime = 0.f;
				mTextLabel->setVisible(false);
				mVoteType = VT_None;
			}
		}
	}

	if (mVoteType == VT_None)
	{
		mCloseTime = 0.f;
		mTextLabel->setVisible(false);
		mVoteType = VT_None;
	}
}

void Voting::updateVoteOptionText(bool canCancel)
{
	std::string str;
	str = "---Voting---\n";
	if (!mVoteText.empty())
		str += mVoteText + "\n";
	for (size_t i = 0; i < mVoteOptions.size(); ++i)
	{
		std::stringstream ss;
		ss << (i + 1);

		std::stringstream ss1;
		ss1 << mVoteOptions[i].numVotes;

		if (!mHasVoted)
		{
			str += "(" + ss.str() + ") ";
		}
		str += mVoteOptions[i].name;

		if (!mIsSelecting)
			str += " (" + ss1.str() + ")";
		str += "\n";
	}
	if (canCancel)
		str += "(0) Cancel";

	mTextLabel->setString(str);
}

void Voting::onMessage(NetMessage& msg)
{
	if (msg.mMsgId == NetMessage::MSG_Voting)
		onVoteMessage(msg);
}

void Voting::onVoteMessage(NetMessage& msg)
{
	std::uint16_t clientId = msg.mClientId;

	std::uint8_t voteType = VT_None;
	std::uint8_t voteOperation = VO_None;

	msg.readUByte(voteType);
	msg.readUByte(voteOperation);

	if (voteOperation == VO_Start)
	{
		std::string playerName;
		msg.readString(playerName);

		mVoteOptions.clear();
		{
			VoteOption yesOption;
			yesOption.name = "Yes";
			yesOption.numVotes = 1;
			mVoteOptions.push_back(yesOption);
		}
		{
			VoteOption noOption;
			noOption.name = "No";
			mVoteOptions.push_back(noOption);
		}

		mVoteText = "Kick Player: " + playerName;
		updateVoteOptionText();
		mTextLabel->setVisible(true);

		mVoteType = (VoteType)voteType;
		mIsSelecting = false;
		mHasVoted = false;
	}
	else if (voteType == mVoteType)
	{
		if (voteOperation == VO_Vote)
		{
			if (mGame->mNetwork->isServer())
			{
				auto it = mClientsHasVoted.find(clientId);
				if (it != mClientsHasVoted.end())
					return;
				mClientsHasVoted.insert(clientId);

				std::uint8_t option = 0;
				msg.readUByte(option);

				if (option < mVoteOptions.size())
				{
					VoteOption& voteOption = mVoteOptions[option];
					voteOption.numVotes++;

					updateVoteOptionText();

					NetMessage voteMsg(NetMessage::MSG_Voting);
					voteMsg.writeUByte(mVoteType);
					voteMsg.writeUByte(VO_Status);
					std::uint8_t size = mVoteOptions.size();
					voteMsg.writeUByte(size);
					for (std::uint8_t i = 0; i < size; ++i)
					{
						voteMsg.writeUByte(mVoteOptions[i].numVotes);
					}

					mGame->mNetwork->send(voteMsg);

					onVoteOver();
				}
			}
		}
		else if (voteOperation == VO_Status)
		{
			if (mGame->mNetwork->isClient())
			{
				std::uint8_t size = 0;
				msg.readUByte(size);

				for (std::uint8_t i = 0; i < size; ++i)
				{
					std::uint8_t numVotes;
					msg.readUByte(numVotes);
					mVoteOptions[i].numVotes = numVotes;
				}

				updateVoteOptionText();
			}
		}
		else if (voteOperation == VO_Accepted)
		{
			mCloseTime = 0.f;
			mTextLabel->setVisible(false);
			mVoteType = VT_None;
		}
		else if (voteOperation == VO_Aborted)
		{
			mCloseTime = 0.f;
			mTextLabel->setVisible(false);
			mVoteType = VT_None;
		}
		else if (voteOperation == VO_Failed)
		{
			mCloseTime = 0.f;
			mTextLabel->setVisible(false);
			mVoteType = VT_None;
		}
	}
}

void Voting::update(float deltaTime)
{
	if (mGame->mNetwork->isServer())
	{
		if (mCloseTime > 0.f)
		{
			mCloseTime -= deltaTime;

			if (mCloseTime <= 0.f)
			{
				onVoteOver();
			}
		}
	}
}