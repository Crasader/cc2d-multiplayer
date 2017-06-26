bool pawnCoin = false;

float coinSpawnTimer = 0;

int restCoins = 5;

array<uint32> playerStarts;

array<uint16> players;
array<int> countCollects;

void onTriggerContactStart(uint32 owner, uint32 obj, const ContactInfo &in contact)
{
	if(!isServer())
		return;

	if(getObjectType(obj) == OT_Player)
	{
		destroyObject(owner);

		coinSpawnTimer = 3.0;
		restCoins--;
		setScore(0, restCoins);

		uint16 clientId = getClientIdFromPlayer(obj);
		if(clientId != gInvalidClientId)
		{
			int idx = players.find(clientId);
			if(idx >= 0)
			{
				countCollects[idx] = countCollects[idx] + 1;
			}
		}

		if(restCoins <= 0)
		{
			int maxCollect = 0;
			int maxCollectIdx = -1;
			for(uint32 i = 0; i < countCollects.length(); i++)
			{
				if(countCollects[i] > maxCollect)
				{
					maxCollect = countCollects[i];
					maxCollectIdx = i;
				}
			}

			if(maxCollectIdx >= 0)
			{
				scoreUser(players[maxCollectIdx]);
			}

			setGameOver();
		}
	}
}

void spawnCoin()
{
	if(!isServer())
		return;

	array<uint32> arr = findObjectsIndexByType(OT_TargetPoint);

	int randomTargetPointNumber = randomInt(0, arr.length() - 1);

	Vec2 spawnPos = getObjectPosition(arr[randomTargetPointNumber]);

	Parameters p;
	uint32 coinIdx = spawnObject("coin", spawnPos, p);

	addObjectEvent(coinIdx, "contactEnter", "onTriggerContactStart");
}

void onClientConnected(uint16 clientId)
{
	players.insertLast(clientId);
	countCollects.insertLast(0);
}

void onClientDisconnected(uint16 clientId)
{
	int idx = players.find(clientId);
	players.removeAt(idx);
	countCollects.removeAt(idx);
}

void onStart()
{
	if(!isServer())
		return;

	initScore(1);
	setScore(0, restCoins);

        playerStarts = findObjectsIndexByType(OT_PlayerStart);
}

void restart()
{
	if(!isServer())
		return;
}

void onAllClientsLoadingFinished()
{
	if(!isServer())
		return;

	checkPlayerRespawn(0);

	coinSpawnTimer = 5.0;
}

Vec2 getPlayerSpawnSpot(uint16 clientId)
{
	Vec2 pos;

	if(playerStarts.length() == 0)
	{
		playerStarts = findObjectsIndexByType(OT_PlayerStart);
	}

	if(playerStarts.length() >= 1)
	{
		pos = getObjectPosition(playerStarts[0]);
		playerStarts.removeAt(0);
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

	if(coinSpawnTimer > 0.0)
	{
		coinSpawnTimer -= deltaTime;

		showGameTimer(true);
		setGameTimer(int(coinSpawnTimer) + 1);

		if(coinSpawnTimer <= 0.0)
		{
			showGameTimer(false);

			coinSpawnTimer = 0.0;
			spawnCoin();
		}
	}

	checkPlayerRespawn(5000);
}