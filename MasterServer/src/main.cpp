#include <iostream>
#include <vector>
#include <memory>

#include <enet/enet.h>
#include <enet/time.h>

#include "MasterServer.h"

int main()
{
	MasterServer masterServer;

	if (masterServer.start())
	{
		masterServer.run();
	}

	system("pause");

	return 0;
}