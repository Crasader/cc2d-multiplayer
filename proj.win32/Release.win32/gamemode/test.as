void onStart()
{
	if(!isServer())
		return;
	
	
	Vec2 spawnPos;
	Parameters p;
	p.setString("name", "ball");
	p.setString("sprite", "Ball.png");
	p.setFloat("scaleX", 1.0);
	p.setFloat("scaleY", 1.0);
	p.setFloat("radius", 1.0);
	p.setFloat("offsetY", 0.0);
	p.setFloat("restitution", 0.0);
	p.setBool("centerImpulse", false);
	p.setBool("isKinematicOnClient", true);
	for(int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 39; x++)
		{
			spawnPos.x = 8.0 + (x * 2.0);
			spawnPos.y = 1.0 + (y * 2.0);

			uint32 ballIdx = spawnObject("ball", spawnPos, p);
		}
	}
}

void onAllClientsLoadingFinished()
{
	if(!isServer())
		return;

	checkPlayerRespawn(0);
}

void update(float deltaTime)
{
	if(!isServer() || isGameOver())
		return;

	checkPlayerRespawn(0);
}