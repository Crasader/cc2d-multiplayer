
enum eTeam
{
	TeamNone = -1,
	TeamLeft = 0,
	TeamRight = 1
}

class Team
{
	int score;
};

array<Team> teams(2);

float resetTimer = 0;
bool isResetting = false;

int maxScore = 2;

array<uint32> leftPlayerStarts;
array<uint32> rightPlayerStarts;

void onTriggerContactStart(uint32 owner, uint32 obj, const ContactInfo &in contact)
{
	if(!isServer() || isGameOver())
		return;

	if(isResetting)
		return;
	if(getObjectType(obj) == OT_Ball)
	{
		if(getObjectName(owner) == "RightGoal")
		{
			teams[TeamLeft].score++;
			setScore(TeamLeft, teams[TeamLeft].score);

			if(teams[TeamLeft].score >= maxScore)
			{
				scoreTeam(TeamLeft);
				setGameOver();
			}
		}
		else if(getObjectName(owner) == "LeftGoal")
		{
			teams[TeamRight].score++;
			setScore(TeamRight, teams[TeamRight].score);

			if(teams[TeamRight].score >= maxScore)
			{
				scoreTeam(TeamRight);
				setGameOver();
			}
		}

		resetTimer = 5.0;
		isResetting = true;
	}
}

void onClientConnected(uint16 clientId)
{
}

void onClientDisconnected(uint16 clientId)
{
}

void onStart()
{
	if(!isServer())
		return;

	uint32 triggerLeftIdx = getObjectIndexByName("LeftGoal");
	uint32 triggerRightIdx = getObjectIndexByName("RightGoal");

	addObjectEvent(triggerLeftIdx, "contactEnter", "onTriggerContactStart");
	addObjectEvent(triggerRightIdx, "contactEnter", "onTriggerContactStart");

	initScore(2);

	teams[TeamLeft].score = 0;
	teams[TeamRight].score = 0;

	float middlePosX = (gLevelWidth / 2.0) / 32.0;

	array<uint32> arr = findObjectsIndexByType(OT_PlayerStart);
	for (uint32 i = 0; i < arr.length(); i++)
	{
		if(getObjectPosition(arr[i]).x >= middlePosX)
			rightPlayerStarts.insertLast(arr[i]);
		else
			leftPlayerStarts.insertLast(arr[i]);
	}
}

void restart()
{
	if(!isServer())
		return;

	uint32 ballSpawnPointIdx = getObjectIndexByName("BallSpawnPoint");
	Vec2 spawnPos = getObjectPosition(ballSpawnPointIdx);
	uint32 ballIdx = getObjectIndexByName("ball");
	setObjectPosition(ballIdx, spawnPos);
	setObjectRotation(ballIdx, 0.0);
	setBodyAwake(ballIdx, false);
}

void onAllClientsLoadingFinished()
{
	if(!isServer())
		return;

	checkPlayerRespawn(0);

	uint32 ballSpawnPointIdx = getObjectIndexByName("BallSpawnPoint");
	Vec2 spawnPos = getObjectPosition(ballSpawnPointIdx);

	Parameters p;
	p.setString("name", "ball");
	p.setString("sprite", "Ball.png");
	p.setFloat("scaleX", 1.0);
	p.setFloat("scaleY", 1.0);
	p.setFloat("radius", 1.0);
	p.setFloat("offsetY", 0.0);
	p.setFloat("maxVelocity", 60.0);
	p.setFloat("impulseX", 30.0);
	p.setFloat("impulseY", 40.0);
	p.setBool("centerImpulse", false);
	uint32 ball = spawnObject("ball", spawnPos, p);
	setBodyAwake(ball, false);
}

Vec2 getPlayerSpawnSpot(uint16 clientId)
{
	Vec2 pos;

	int team = getTeam(clientId);

	if(team == TeamNone)
	{
		uint32 leftPlayer = getNumClientsInTeam(TeamLeft);
		uint32 rightPlayer = getNumClientsInTeam(TeamRight);
		if(leftPlayer == rightPlayer)
		{
			team = randomInt(0, 1);
		}
		else if(leftPlayer > rightPlayer)
		{
			team = TeamRight;
		}
		else
		{
			team = TeamLeft;
		}
		setTeam(clientId, team);
	}

	if(team == TeamRight)
	{
		if(rightPlayerStarts.length() == 0)
		{
			float middlePosX = (gLevelWidth / 2.0) / 32.0;
			array<uint32> arr = findObjectsIndexByType(OT_PlayerStart);
			for (uint32 i = 0; i < arr.length(); i++)
			{
				if(getObjectPosition(arr[i]).x >= middlePosX)
					rightPlayerStarts.insertLast(arr[i]);
			}
		}
		if(rightPlayerStarts.length() >= 1)
		{
			pos = getObjectPosition(rightPlayerStarts[0]);
			rightPlayerStarts.removeAt(0);
		}
	}
	else if(team == TeamLeft)
	{
		if(leftPlayerStarts.length() == 0)
		{
			float middlePosX = (gLevelWidth / 2.0) / 32.0;
			array<uint32> arr = findObjectsIndexByType(OT_PlayerStart);
			for (uint32 i = 0; i < arr.length(); i++)
			{
				if(getObjectPosition(arr[i]).x < middlePosX)
					leftPlayerStarts.insertLast(arr[i]);
			}
		}
		if(leftPlayerStarts.length() >= 1)
		{
			pos = getObjectPosition(leftPlayerStarts[0]);
			leftPlayerStarts.removeAt(0);
		}
	}

	return pos;
}

void onSpawnPlayer(uint32 index)
{
}

void onCharacterDestroyed(uint32 index)
{
}

void update(float deltaTime)
{
	if(!isServer() || isGameOver())
		return;

	if(resetTimer > 0.0)
	{
		resetTimer -= deltaTime;

		showGameTimer(true);
		setGameTimer(int(resetTimer) + 1);

		if(resetTimer <= 0.0)
		{
			resetTimer = 0.0;
			isResetting = false;

			showGameTimer(false);

			restart();
		}
	}

	checkPlayerRespawn(5000);
}