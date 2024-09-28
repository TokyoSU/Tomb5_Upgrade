#include "../tomb5/pch.h"
#include "configuration.h"
#include <SimpleIni.h>
#include "function_stubs.h"
#include "input.h"
#include "winmain.h"
#include "LoadSave.h"
#include "tomb5/tomb5.h"

extern bool fmvs_disabled;
CConfig g_Config;

constexpr auto CONFIG_FILENAME = "config.ini";

constexpr auto SECTION_SYSTEM = "Tomb5_System";
constexpr auto SECTION_GAME = "Tomb5_Game";
constexpr auto SECTION_TOMB5_STUFF = "Tomb5_Stuff";

void CConfig::Initialize()
{
	// Setup default values.
	haveDoneSetup = false;

	tomb5.footprints = 1;
	tomb5.shadow_mode = 3;
	tomb5.fix_climb_up_delay = 1;
	tomb5.flexible_crawling = 1;
	tomb5.cutseq_skipper = 0;
	tomb5.enable_cheats = 0;
	tomb5.bars_pos = 1;
	tomb5.enemy_bars = 0;
	tomb5.ammo_counter = 0;
	tomb5.gameover = 1;
	tomb5.fog = 1;
	tomb5.bar_mode = 3;
	tomb5.crawltilt = 1;
	tomb5.PSX_skies = 1;
	tomb5.tr4_loadscreens = 1;
	tomb5.tr4_loadbar = 1;
	tomb5.inv_bg_mode = 1;
	tomb5.loadingtxt = 1;
	tomb5.shimmer = 1;
	tomb5.distance_fog = 12;
	tomb5.ammotype_hotkeys = 1;
	tomb5.look_transparency = 0;
	tomb5.static_lighting = 1;
	tomb5.uw_dust = 2;

	// Now load file.
	CSimpleIniA ini;
	ini.SetUnicode();
	SI_Error err = ini.LoadFile(CONFIG_FILENAME);
	if (err < SI_OK)
	{
		Log("Failed to load <%s>, saving default setting and stuff !", CONFIG_FILENAME);
		SaveDummy();
		SaveSetting(true);
		SaveStuff();
	}
	LoadSetting();
	LoadStuff();
}

void CConfig::LoadSetting()
{
	CSimpleIniA ini;
	SI_Error err = ini.LoadFile(CONFIG_FILENAME);
	if (err < SI_OK)
	{
		Log("Failed to load <%s>, settings not loaded !", CONFIG_FILENAME);
		return;
	}

	haveDoneSetup = ini.GetBoolValue(SECTION_SYSTEM, "HaveDoneSetup", false);
	if (haveDoneSetup)
	{
		App.TextureSize = 256;
		App.BumpMapSize = 256;
		App.BumpMapping = false;
		App.StartFlags = DXF_FPUSETUP | DXF_HWR | DXF_ZBUFFER;

		// Load SYSTEM
		App.DXInfo.nDD = ini.GetLongValue(SECTION_SYSTEM, "DD");
		App.DXInfo.nD3D = ini.GetLongValue(SECTION_SYSTEM, "D3D");
		App.DXInfo.nDisplayMode = ini.GetLongValue(SECTION_SYSTEM, "VMode");
		App.DXInfo.nTexture = ini.GetLongValue(SECTION_SYSTEM, "TFormat");
		App.DXInfo.nDS = ini.GetLongValue(SECTION_SYSTEM, "DS");

		App.Filtering = ini.GetBoolValue(SECTION_SYSTEM, "Filter", true);
		App.SoundDisabled = ini.GetBoolValue(SECTION_SYSTEM, "DisableSound");
		App.Volumetric = ini.GetBoolValue(SECTION_SYSTEM, "Volumetric", true);
		fmvs_disabled = ini.GetBoolValue(SECTION_SYSTEM, "NoFMV");

		App.dx.rScreen.left = ini.GetLongValue(SECTION_SYSTEM, "WindowX");
		App.dx.rScreen.top = ini.GetLongValue(SECTION_SYSTEM, "WindowY");
		if (ini.GetBoolValue(SECTION_SYSTEM, "TextLow"))
			App.TextureSize = 128;

		if (ini.GetBoolValue(SECTION_SYSTEM, "Windowed"))
			App.StartFlags |= DXF_WINDOWED;
		else
			App.StartFlags |= DXF_FULLSCREEN;
	}


	// Load GAME
	/// Player keyboard
	for (int i = 0; i < LAYK_Count; i++)
	{
		layout[LAY_Player][i] = (SDL_Scancode)ini.GetLongValue(SECTION_GAME, std::string("Key" + std::to_string(i)).c_str());
		layout[LAY_Default][i] = layout[LAY_Player][i];
	}

	/// Now joystick
	jLayout[0] = ini.GetLongValue(SECTION_GAME, "JDck", 5);
	jLayout[1] = ini.GetLongValue(SECTION_GAME, "JDsh", 3);
	jLayout[2] = ini.GetLongValue(SECTION_GAME, "JWlk", 4);
	jLayout[3] = ini.GetLongValue(SECTION_GAME, "JJmp", 0);
	jLayout[4] = ini.GetLongValue(SECTION_GAME, "JAct", 1);
	jLayout[5] = ini.GetLongValue(SECTION_GAME, "JDrw", 2);
	jLayout[6] = ini.GetLongValue(SECTION_GAME, "JFlr", 9);
	jLayout[7] = ini.GetLongValue(SECTION_GAME, "JLok", 6);
	jLayout[8] = ini.GetLongValue(SECTION_GAME, "JRol", 7);
	jLayout[9] = ini.GetLongValue(SECTION_GAME, "JInv", 8);
	ControlMethod = (ControlMethodType)ini.GetLongValue(SECTION_GAME, "ControlMethod", 0);
	MusicVolume = ini.GetLongValue(SECTION_GAME, "MusicVolume", 100);
	SFXVolume = ini.GetLongValue(SECTION_GAME, "SFXVolume", 100);
	SoundQuality = ini.GetLongValue(SECTION_GAME, "SoundQuality", 1);
	App.AutoTarget = ini.GetLongValue(SECTION_GAME, "AutoTarget", 1);
}

void CConfig::SaveSetting(bool isFirst)
{
	CSimpleIniA ini;
	SI_Error err;

	if (!isFirst)
	{
		err = ini.LoadFile(CONFIG_FILENAME);
		if (err < SI_OK)
		{
			Log("Failed to load <%s>, settings not saved !", CONFIG_FILENAME);
			return;
		}
	}
	
	// Saving GAME
	/// Player keyboard
	for (int i = 0; i < LAYK_Count; i++)
		ini.SetLongValue(SECTION_GAME, std::string("Key" + std::to_string(i)).c_str(), (long)layout[LAY_Player][i]);

	/// Now joystick
	ini.SetLongValue(SECTION_GAME, "JDck", jLayout[0]);
	ini.SetLongValue(SECTION_GAME, "JDsh", jLayout[1]);
	ini.SetLongValue(SECTION_GAME, "JWlk", jLayout[2]);
	ini.SetLongValue(SECTION_GAME, "JJmp", jLayout[3]);
	ini.SetLongValue(SECTION_GAME, "JAct", jLayout[4]);
	ini.SetLongValue(SECTION_GAME, "JDrw", jLayout[5]);
	ini.SetLongValue(SECTION_GAME, "JFlr", jLayout[6]);
	ini.SetLongValue(SECTION_GAME, "JLok", jLayout[7]);
	ini.SetLongValue(SECTION_GAME, "JRol", jLayout[8]);
	ini.SetLongValue(SECTION_GAME, "JInv", jLayout[9]);

	if (!isFirst)
	{
		ini.SetLongValue(SECTION_GAME, "ControlMethod", ControlMethod);
		ini.SetLongValue(SECTION_GAME, "MusicVolume", MusicVolume);
		ini.SetLongValue(SECTION_GAME, "SFXVolume", SFXVolume);
		ini.SetLongValue(SECTION_GAME, "SoundQuality", SoundQuality);
		ini.SetLongValue(SECTION_GAME, "AutoTarget", App.AutoTarget);
	}

	// Saving SYSTEM
	ini.SetLongValue(SECTION_SYSTEM, "VMode", App.DXInfo.nDisplayMode);
	ini.SetLongValue(SECTION_SYSTEM, "WindowX", App.dx.rScreen.left);
	ini.SetLongValue(SECTION_SYSTEM, "WindowY", App.dx.rScreen.top);
	ini.SetBoolValue(SECTION_SYSTEM, "Windowed", (App.dx.Flags & DXF_WINDOWED) != 0);

	// Now save the file.
	err = ini.SaveFile(CONFIG_FILENAME);
	if (err < SI_OK)
	{
		Log("Failed to save <%s>, settings not saved !", CONFIG_FILENAME);
	}
}

void CConfig::LoadStuff()
{
	CSimpleIniA ini;
	SI_Error err = ini.LoadFile(CONFIG_FILENAME);
	if (err < SI_OK)
	{
		Log("Failed to load <%s>, stuff not loaded !", CONFIG_FILENAME);
		return;
	}
	tomb5.footprints = ini.GetBoolValue(SECTION_TOMB5_STUFF, "footprints");
	tomb5.shadow_mode = ini.GetLongValue(SECTION_TOMB5_STUFF, "shadow");
	tomb5.fix_climb_up_delay = ini.GetBoolValue(SECTION_TOMB5_STUFF, "climbup");
	tomb5.flexible_crawling = ini.GetBoolValue(SECTION_TOMB5_STUFF, "flex_crawl");
	tomb5.cutseq_skipper = ini.GetBoolValue(SECTION_TOMB5_STUFF, "cutseq_skipper");
	tomb5.enable_cheats = ini.GetBoolValue(SECTION_TOMB5_STUFF, "cheats");
	tomb5.bars_pos = ini.GetLongValue(SECTION_TOMB5_STUFF, "bar_pos");
	tomb5.enemy_bars = ini.GetBoolValue(SECTION_TOMB5_STUFF, "enemy_bar");
	tomb5.ammo_counter = ini.GetBoolValue(SECTION_TOMB5_STUFF, "ammo_counter");
	tomb5.gameover = ini.GetBoolValue(SECTION_TOMB5_STUFF, "gameover");
	tomb5.fog = ini.GetBoolValue(SECTION_TOMB5_STUFF, "fog");
	tomb5.bar_mode = ini.GetLongValue(SECTION_TOMB5_STUFF, "barMode");
	tomb5.crawltilt = ini.GetBoolValue(SECTION_TOMB5_STUFF, "crawltilt");
	tomb5.PSX_skies = ini.GetBoolValue(SECTION_TOMB5_STUFF, "psxsky");
	tomb5.tr4_loadscreens = ini.GetBoolValue(SECTION_TOMB5_STUFF, "tr4LS");
	tomb5.tr4_loadbar = ini.GetBoolValue(SECTION_TOMB5_STUFF, "tr4LB");
	tomb5.inv_bg_mode = ini.GetLongValue(SECTION_TOMB5_STUFF, "inv_bgM");
	tomb5.loadingtxt = ini.GetBoolValue(SECTION_TOMB5_STUFF, "loadtxt");
	tomb5.shimmer = ini.GetBoolValue(SECTION_TOMB5_STUFF, "shimmer");
	tomb5.distance_fog = ini.GetLongValue(SECTION_TOMB5_STUFF, "distance_fog");
	tomb5.ammotype_hotkeys = ini.GetBoolValue(SECTION_TOMB5_STUFF, "ammotype_hotkeys");
	tomb5.look_transparency = ini.GetBoolValue(SECTION_TOMB5_STUFF, "ltransparency");
	tomb5.static_lighting = ini.GetBoolValue(SECTION_TOMB5_STUFF, "static_lighting");
	tomb5.uw_dust = ini.GetLongValue(SECTION_TOMB5_STUFF, "uw_dust");
}

void CConfig::SaveStuff(bool isFirst)
{
	CSimpleIniA ini;
	SI_Error err;

	if (!isFirst)
	{
		err = ini.LoadFile(CONFIG_FILENAME);
		if (err < SI_OK)
		{
			Log("Failed to load <%s>, stuff not saved !", CONFIG_FILENAME);
			return;
		}
	}
	
	// Writing stuff now.
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "footprints", tomb5.footprints);
	ini.SetLongValue(SECTION_TOMB5_STUFF, "shadow", tomb5.shadow_mode);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "climbup", tomb5.fix_climb_up_delay);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "flex_crawl", tomb5.flexible_crawling);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "cutseq_skipper", tomb5.cutseq_skipper);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "cheats", tomb5.enable_cheats);
	ini.SetLongValue(SECTION_TOMB5_STUFF, "bar_pos", tomb5.bars_pos);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "enemy_bar", tomb5.enemy_bars);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "ammo_counter", tomb5.ammo_counter);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "gameover", tomb5.gameover);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "fog", tomb5.fog);
	ini.SetLongValue(SECTION_TOMB5_STUFF, "barMode", tomb5.bar_mode);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "crawltilt", tomb5.crawltilt);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "psxsky", tomb5.PSX_skies);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "tr4LS", tomb5.tr4_loadscreens);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "tr4LB", tomb5.tr4_loadbar);
	ini.SetLongValue(SECTION_TOMB5_STUFF, "inv_bgM", tomb5.inv_bg_mode);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "loadtxt", tomb5.loadingtxt);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "shimmer", tomb5.shimmer);
	ini.SetLongValue(SECTION_TOMB5_STUFF, "distance_fog", tomb5.distance_fog);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "ammotype_hotkeys", tomb5.ammotype_hotkeys);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "ltransparency", tomb5.look_transparency);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "static_lighting", tomb5.static_lighting);
	ini.SetLongValue(SECTION_TOMB5_STUFF, "uw_dust", tomb5.uw_dust);

	// Now save the file.
	err = ini.SaveFile(CONFIG_FILENAME);
	if (err < SI_OK)
	{
		Log("Failed to save <%s>, stuff not saved !", CONFIG_FILENAME);
	}
}

void CConfig::SaveSetup(HWND hDlg, bool isFirst)
{
	CSimpleIniA ini;
	SI_Error err;

	if (!isFirst)
	{
		err = ini.LoadFile(CONFIG_FILENAME);
		if (err < SI_OK)
		{
			Log("Failed to load <%s>, setup setting not saved !", CONFIG_FILENAME);
			return;
		}
	}
	
	// Saving SYSTEM from setup.
	ini.SetLongValue(SECTION_SYSTEM, "DD", SendMessage(GetDlgItem(hDlg, IDC_GRAPHICS_ADAPTER), CB_GETCURSEL, 0, 0));
	ini.SetLongValue(SECTION_SYSTEM, "D3D", SendMessage(GetDlgItem(hDlg, IDC_D3D), CB_GETCURSEL, 0, 0));
	ini.SetLongValue(SECTION_SYSTEM, "VMode", SendMessage(GetDlgItem(hDlg, IDC_RESOLUTION), CB_GETITEMDATA, SendMessage(GetDlgItem(hDlg, IDC_RESOLUTION), CB_GETCURSEL, 0, 0), 0));
	ini.SetLongValue(SECTION_SYSTEM, "DS", SendMessage(GetDlgItem(hDlg, IDC_SOUND_ADAPTER), CB_GETCURSEL, 0, 0));
	ini.SetLongValue(SECTION_SYSTEM, "TFormat", SendMessage(GetDlgItem(hDlg, IDC_TFORMAT), CB_GETCURSEL, 0, 0));
	ini.SetBoolValue(SECTION_SYSTEM, "Filter", SendMessage(GetDlgItem(hDlg, IDC_BILINEAR), BM_GETCHECK, 0, 0));
	ini.SetBoolValue(SECTION_SYSTEM, "DisableSound", SendMessage(GetDlgItem(hDlg, IDC_DISABLE_SOUND), BM_GETCHECK, 0, 0));
	ini.SetBoolValue(SECTION_SYSTEM, "TextLow", SendMessage(GetDlgItem(hDlg, IDC_LOW_QUALITY_TEX), BM_GETCHECK, 0, 0));
	ini.SetBoolValue(SECTION_SYSTEM, "Windowed", SendMessage(GetDlgItem(hDlg, IDC_WINDOWED), BM_GETCHECK, 0, 0));
	ini.SetBoolValue(SECTION_SYSTEM, "NoFMV", SendMessage(GetDlgItem(hDlg, IDC_DISABLE_FMV), BM_GETCHECK, 0, 0));
	ini.SetBoolValue(SECTION_SYSTEM, "HaveDoneSetup", true);

	// Now save the file.
	err = ini.SaveFile(CONFIG_FILENAME);
	if (err < SI_OK)
	{
		Log("Failed to save <%s>, setup setting not saved !", CONFIG_FILENAME);
	}
}

void CConfig::SaveDummy()
{	
	CSimpleIniA ini;

	// Game
	for (int i = 0; i < LAYK_Count; i++)
		ini.SetLongValue(SECTION_GAME, std::string("Key" + std::to_string(i)).c_str(), 0);
	ini.SetLongValue(SECTION_GAME, "JDck", 5);
	ini.SetLongValue(SECTION_GAME, "JDsh", 3);
	ini.SetLongValue(SECTION_GAME, "JWlk", 4);
	ini.SetLongValue(SECTION_GAME, "JJmp", 0);
	ini.SetLongValue(SECTION_GAME, "JAct", 1);
	ini.SetLongValue(SECTION_GAME, "JDrw", 2);
	ini.SetLongValue(SECTION_GAME, "JFlr", 9);
	ini.SetLongValue(SECTION_GAME, "JLok", 6);
	ini.SetLongValue(SECTION_GAME, "JRol", 7);
	ini.SetLongValue(SECTION_GAME, "JInv", 8);
	ini.SetLongValue(SECTION_GAME, "ControlMethod", 0);
	ini.SetLongValue(SECTION_GAME, "MusicVolume", 90);
	ini.SetLongValue(SECTION_GAME, "SFXVolume", 90);
	ini.SetLongValue(SECTION_GAME, "SoundQuality", 1);
	ini.SetLongValue(SECTION_GAME, "AutoTarget", 1);

	// System
	ini.SetLongValue(SECTION_SYSTEM, "DD", 0);
	ini.SetLongValue(SECTION_SYSTEM, "D3D", 0);
	ini.SetLongValue(SECTION_SYSTEM, "VMode", 0);
	ini.SetLongValue(SECTION_SYSTEM, "DS", 0);
	ini.SetLongValue(SECTION_SYSTEM, "TFormat", 0);
	ini.SetBoolValue(SECTION_SYSTEM, "Filter", false);
	ini.SetBoolValue(SECTION_SYSTEM, "DisableSound", false);
	ini.SetBoolValue(SECTION_SYSTEM, "TextLow", false);
	ini.SetBoolValue(SECTION_SYSTEM, "Windowed", false);
	ini.SetBoolValue(SECTION_SYSTEM, "NoFMV", false);
	ini.SetBoolValue(SECTION_SYSTEM, "HaveDoneSetup", false);

	// Stuff
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "footprints", false);
	ini.SetLongValue(SECTION_TOMB5_STUFF, "shadow", 0);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "climbup", false);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "flex_crawl", false);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "cutseq_skipper", false);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "cheats", false);
	ini.SetLongValue(SECTION_TOMB5_STUFF, "bar_pos", 0);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "enemy_bar", false);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "ammo_counter", false);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "gameover", false);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "fog", false);
	ini.SetLongValue(SECTION_TOMB5_STUFF, "barMode", 0);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "crawltilt", false);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "psxsky", false);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "tr4LS", false);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "tr4LB", false);
	ini.SetLongValue(SECTION_TOMB5_STUFF, "inv_bgM", 0);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "loadtxt", false);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "shimmer", false);
	ini.SetLongValue(SECTION_TOMB5_STUFF, "distance_fog", 0);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "ammotype_hotkeys", false);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "ltransparency", false);
	ini.SetBoolValue(SECTION_TOMB5_STUFF, "static_lighting", false);
	ini.SetLongValue(SECTION_TOMB5_STUFF, "uw_dust", 0);

	// Now save the file.
	SI_Error err = ini.SaveFile(CONFIG_FILENAME);
	if (err < SI_OK)
	{
		Log("Failed to save <%s>, setup setting not saved !", CONFIG_FILENAME);
	}
}
