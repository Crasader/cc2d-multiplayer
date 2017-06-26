#ifndef __VOTING_H__
#define __VOTING_H__

#include "NetService.h"

#include "cocos2d.h"

class GameSystem;

struct VoteOption
{
	std::string name;
	std::uint32_t numVotes = 0;
};

class Voting : public NetService, public cocos2d::Layer
{
public:
	enum VoteType
	{
		VT_None,
		VT_KickPlayer,
		VT_MapChange,
	};

	enum VoteOperation
	{
		VO_None,
		VO_Start,
		VO_Vote,
		VO_Status,
		VO_Accepted,
		VO_Aborted,
		VO_Failed,
	};

public:
	Voting(GameSystem* game);
	~Voting();

	void reset();

	void startVote();
	bool isVisible() const;

	void vote(VoteType type, std::uint8_t option);

	void onCancel();
	void onOption(std::uint8_t option);
	void onOption1();
	void onOption2();
	void onOption3();
	void onOption4();
	void onOption5();
	void onOption6();
	void onOption7();
	void onOption8();
	void onOption9();

	void onVoteOver();

	void updateVoteOptionText(bool canCancel = false);

	virtual void onMessage(NetMessage& msg) override;
	virtual void onVoteMessage(NetMessage& msg);
	virtual void update(float deltaTime) override;

	GameSystem* mGame = nullptr;

	cocos2d::Label* mTextLabel = nullptr;

	float mCloseTime = 0;

	bool mHasVoted = false;

	VoteType mVoteType = VT_None;
	std::vector<VoteOption> mVoteOptions;

	std::string mVoteText;

	std::set<std::uint16_t> mClientsHasVoted;

	bool mIsSelecting = false;

	int mNumTotalVotes = 0;
	std::string mVoteName;
};

#endif