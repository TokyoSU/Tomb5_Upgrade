#include "../tomb5/pch.h"
#include "tomb5.h"
#include "../specific/registry.h"
#include "../game/gameflow.h"
#include "../game/lara.h"
#include "../specific/configuration.h"

tomb5_options tomb5;

void init_tomb5_stuff()
{
	g_Config.LoadStuff();
}

void save_tomb5_stuff()
{
	g_Config.SaveStuff();
}
