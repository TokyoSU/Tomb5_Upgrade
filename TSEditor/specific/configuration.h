#pragma once

struct CConfig
{
	bool haveDoneSetup;                 //on off

	void Initialize();
	void LoadSetting();
	void SaveSetting(bool isFirst = false);
	void LoadStuff();
	void SaveStuff(bool isFirst = false);
	void SaveSetup(HWND hDlg, bool isFirst = false);
	void SaveDummy();
};

extern CConfig g_Config;
