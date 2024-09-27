#include "../tomb5/pch.h"
#include "registry.h"
#include "LoadSave.h"
#include "winmain.h"
#include "input.h"
#include "setupdlg.h"
#include "configuration.h"

bool LoadSettings()
{
	g_Config.LoadSetting();
	CheckKeyConflicts();
	return g_Config.haveDoneSetup;
}

void SaveSettings()
{
	g_Config.SaveSetting();
}

bool SaveSetup(HWND hDlg)
{
	g_Config.SaveSetup(hDlg);
	return true;
}
