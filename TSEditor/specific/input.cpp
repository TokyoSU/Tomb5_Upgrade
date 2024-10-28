#include "../tomb5/pch.h"
#include "input.h"
#include "dxshell.h"
#include "../game/lara_states.h"
#include "../game/sound.h"
#include "3dmath.h"
#include "../game/text.h"
#include "winmain.h"
#include "../game/camera.h"
#include "../game/spotcam.h"
#include "../game/control.h"
#include "LoadSave.h"
#include "../game/lara.h"
#include "../game/savegame.h"
#include "../game/gameflow.h"
#include "../game/newinv2.h"
#include "../tomb5/tomb5.h"

KeyButton KeyboardButtons[] =
{
	KeyButton(SDL_SCANCODE_UNKNOWN, "Unknown"),

	KeyButton(SDL_SCANCODE_A, "A"),
	KeyButton(SDL_SCANCODE_B, "B"),
	KeyButton(SDL_SCANCODE_C, "C"),
	KeyButton(SDL_SCANCODE_D, "D"),
	KeyButton(SDL_SCANCODE_E, "E"),
	KeyButton(SDL_SCANCODE_F, "F"),
	KeyButton(SDL_SCANCODE_G, "G"),
	KeyButton(SDL_SCANCODE_H, "H"),
	KeyButton(SDL_SCANCODE_I, "I"),
	KeyButton(SDL_SCANCODE_J, "J"),
	KeyButton(SDL_SCANCODE_K, "K"),
	KeyButton(SDL_SCANCODE_L, "L"),
	KeyButton(SDL_SCANCODE_M, "M"),
	KeyButton(SDL_SCANCODE_N, "N"),
	KeyButton(SDL_SCANCODE_O, "O"),
	KeyButton(SDL_SCANCODE_P, "P"),
	KeyButton(SDL_SCANCODE_Q, "Q"),
	KeyButton(SDL_SCANCODE_R, "R"),
	KeyButton(SDL_SCANCODE_S, "S"),
	KeyButton(SDL_SCANCODE_T, "T"),
	KeyButton(SDL_SCANCODE_U, "U"),
	KeyButton(SDL_SCANCODE_V, "V"),
	KeyButton(SDL_SCANCODE_W, "W"),
	KeyButton(SDL_SCANCODE_X, "X"),
	KeyButton(SDL_SCANCODE_Y, "Y"),
	KeyButton(SDL_SCANCODE_Z, "Z"),

	KeyButton(SDL_SCANCODE_1, "1"),
	KeyButton(SDL_SCANCODE_2, "2"),
	KeyButton(SDL_SCANCODE_3, "3"),
	KeyButton(SDL_SCANCODE_4, "4"),
	KeyButton(SDL_SCANCODE_5, "5"),
	KeyButton(SDL_SCANCODE_6, "6"),
	KeyButton(SDL_SCANCODE_7, "7"),
	KeyButton(SDL_SCANCODE_8, "8"),
	KeyButton(SDL_SCANCODE_9, "9"),
	KeyButton(SDL_SCANCODE_0, "0"),

	KeyButton(SDL_SCANCODE_RETURN, "Return"),
	KeyButton(SDL_SCANCODE_ESCAPE, "Escape"),
	KeyButton(SDL_SCANCODE_BACKSPACE, "Backspace"),
	KeyButton(SDL_SCANCODE_TAB, "Tab"),
	KeyButton(SDL_SCANCODE_SPACE, "Space"),

	KeyButton(SDL_SCANCODE_MINUS, "Minus"),
	KeyButton(SDL_SCANCODE_EQUALS, "Equals"),
	KeyButton(SDL_SCANCODE_LEFTBRACKET, "LeftBracket"),
	KeyButton(SDL_SCANCODE_RIGHTBRACKET, "RightBacket"),
	KeyButton(SDL_SCANCODE_BACKSLASH, "Backslash"),
	KeyButton(SDL_SCANCODE_NONUSHASH, "NonUsSlash"),
	KeyButton(SDL_SCANCODE_SEMICOLON, "Semicolon"),
	KeyButton(SDL_SCANCODE_APOSTROPHE, "Apostrophe"),
	KeyButton(SDL_SCANCODE_GRAVE, "Grave"),
	KeyButton(SDL_SCANCODE_COMMA, "Comma"),
	KeyButton(SDL_SCANCODE_PERIOD, "Period"),
	KeyButton(SDL_SCANCODE_SLASH, "Slash"),
	KeyButton(SDL_SCANCODE_CAPSLOCK, "Capslock"),

	KeyButton(SDL_SCANCODE_F1, "F1"),
	KeyButton(SDL_SCANCODE_F2, "F2"),
	KeyButton(SDL_SCANCODE_F3, "F3"),
	KeyButton(SDL_SCANCODE_F4, "F4"),
	KeyButton(SDL_SCANCODE_F5, "F5"),
	KeyButton(SDL_SCANCODE_F6, "F6"),
	KeyButton(SDL_SCANCODE_F7, "F7"),
	KeyButton(SDL_SCANCODE_F8, "F8"),
	KeyButton(SDL_SCANCODE_F9, "F9"),
	KeyButton(SDL_SCANCODE_F10, "F10"),
	KeyButton(SDL_SCANCODE_F11, "F11"),
	KeyButton(SDL_SCANCODE_F12, "F12"),
	
	KeyButton(SDL_SCANCODE_PRINTSCREEN, "PrintScreen"),
	KeyButton(SDL_SCANCODE_SCROLLLOCK, "ScrollLock"),
	KeyButton(SDL_SCANCODE_PAUSE, "Pause"),
	KeyButton(SDL_SCANCODE_INSERT, "Insert"),

	KeyButton(SDL_SCANCODE_HOME, "Home"),
	KeyButton(SDL_SCANCODE_PAGEUP, "PageUp"),
	KeyButton(SDL_SCANCODE_DELETE, "Delete"),
	KeyButton(SDL_SCANCODE_END, "End"),
	KeyButton(SDL_SCANCODE_PAGEDOWN, "PageDown"),
	KeyButton(SDL_SCANCODE_RIGHT, "Right"),
	KeyButton(SDL_SCANCODE_LEFT, "Left"),
	KeyButton(SDL_SCANCODE_DOWN, "Down"),
	KeyButton(SDL_SCANCODE_UP, "Up"),

	KeyButton(SDL_SCANCODE_NUMLOCKCLEAR, "NumLockClear"),

	KeyButton(SDL_SCANCODE_KP_DIVIDE, "Numpad Divide"),
	KeyButton(SDL_SCANCODE_KP_MULTIPLY, "Numpad Multiply"),
	KeyButton(SDL_SCANCODE_KP_MINUS, "Numpad Minus"),
	KeyButton(SDL_SCANCODE_KP_PLUS, "Numpad Plus"),
	KeyButton(SDL_SCANCODE_KP_ENTER, "Numpad Enter"),
	KeyButton(SDL_SCANCODE_KP_1, "Numpad 1"),
	KeyButton(SDL_SCANCODE_KP_2, "Numpad 2"),
	KeyButton(SDL_SCANCODE_KP_3, "Numpad 3"),
	KeyButton(SDL_SCANCODE_KP_4, "Numpad 4"),
	KeyButton(SDL_SCANCODE_KP_5, "Numpad 5"),
	KeyButton(SDL_SCANCODE_KP_6, "Numpad 6"),
	KeyButton(SDL_SCANCODE_KP_7, "Numpad 7"),
	KeyButton(SDL_SCANCODE_KP_8, "Numpad 8"),
	KeyButton(SDL_SCANCODE_KP_9, "Numpad 9"),
	KeyButton(SDL_SCANCODE_KP_0, "Numpad 0"),
	KeyButton(SDL_SCANCODE_KP_PERIOD, "Numpad Period"),
	KeyButton(SDL_SCANCODE_KP_EQUALS, "Numpad Equals"),
	KeyButton(SDL_SCANCODE_KP_COMMA, "Keypad Comma"),

	KeyButton(SDL_SCANCODE_NONUSBACKSLASH, "NonUsBackslash"),

	KeyButton(SDL_SCANCODE_APPLICATION, "Application"),
	KeyButton(SDL_SCANCODE_POWER, "Power"),

	KeyButton(SDL_SCANCODE_F13, "F13"),
	KeyButton(SDL_SCANCODE_F14, "F14"),
	KeyButton(SDL_SCANCODE_F15, "F15"),
	KeyButton(SDL_SCANCODE_F16, "F16"),
	KeyButton(SDL_SCANCODE_F17, "F17"),
	KeyButton(SDL_SCANCODE_F18, "F18"),
	KeyButton(SDL_SCANCODE_F19, "F19"),
	KeyButton(SDL_SCANCODE_F20, "F20"),
	KeyButton(SDL_SCANCODE_F21, "F21"),
	KeyButton(SDL_SCANCODE_F22, "F22"),
	KeyButton(SDL_SCANCODE_F23, "F23"),
	KeyButton(SDL_SCANCODE_F24, "F24"),

	KeyButton(SDL_SCANCODE_EXECUTE, "Execute"),
	KeyButton(SDL_SCANCODE_HELP, "Help"),
	KeyButton(SDL_SCANCODE_MENU, "Menu"),
	KeyButton(SDL_SCANCODE_SELECT, "Select"),
	KeyButton(SDL_SCANCODE_STOP, "Stop"),
	KeyButton(SDL_SCANCODE_AGAIN, "Again"),
	KeyButton(SDL_SCANCODE_UNDO, "Undo"),
	KeyButton(SDL_SCANCODE_CUT, "Cut"),
	KeyButton(SDL_SCANCODE_COPY, "Copy"),
	KeyButton(SDL_SCANCODE_PASTE, "Paste"),
	KeyButton(SDL_SCANCODE_FIND, "Find"),
	KeyButton(SDL_SCANCODE_MUTE, "Mute"),
	KeyButton(SDL_SCANCODE_VOLUMEUP, "VolumeUp"),
	KeyButton(SDL_SCANCODE_VOLUMEDOWN, "VolumeDown"),

	KeyButton(SDL_SCANCODE_LCTRL, "Left Control"),
	KeyButton(SDL_SCANCODE_LSHIFT, "Left Shift"),
	KeyButton(SDL_SCANCODE_LALT, "Left Alt"),
	KeyButton(SDL_SCANCODE_LGUI, "Left Gui"),
	KeyButton(SDL_SCANCODE_RCTRL, "Right Control"),
	KeyButton(SDL_SCANCODE_RSHIFT, "Right Shift"),
	KeyButton(SDL_SCANCODE_RALT, "Right Alt"),
	KeyButton(SDL_SCANCODE_RGUI, "Right Gui")
};

KeyButton GermanKeyboard[SDL_NUM_SCANCODES] = {};

const char* JoyStickButtons[16] =
{
	"Joy 1", "Joy 2", "Joy 3", "Joy 4", "Joy 5", "Joy 6", "Joy 7", "Joy 8",
	"Joy 9", "Joy 10", "Joy 11", "Joy 12", "Joy 13", "Joy 14", "Joy 15", "Joy 16"
};

long jLayout[32] = { 5, 3, 4, 0, 1, 2, 9, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
long defaultJLayout[32] = { 5, 3, 4, 0, 1, 2, 9, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

SDL_Scancode layout[LAY_Count][LAYK_Count] =
{
	// Default
	{ SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_D, SDL_SCANCODE_KP_1, SDL_SCANCODE_KP_7, SDL_SCANCODE_LSHIFT, SDL_SCANCODE_KP_4, SDL_SCANCODE_KP_5,
	SDL_SCANCODE_KP_8, SDL_SCANCODE_KP_9, SDL_SCANCODE_KP_0, SDL_SCANCODE_KP_6, SDL_SCANCODE_TAB, SDL_SCANCODE_PAGEUP, SDL_SCANCODE_PAGEDOWN, SDL_SCANCODE_P, SDL_SCANCODE_RETURN },
	// Player
	{ SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_D, SDL_SCANCODE_KP_1, SDL_SCANCODE_KP_7, SDL_SCANCODE_LSHIFT, SDL_SCANCODE_KP_4, SDL_SCANCODE_KP_5,
	SDL_SCANCODE_KP_8, SDL_SCANCODE_KP_9, SDL_SCANCODE_KP_0, SDL_SCANCODE_KP_6, SDL_SCANCODE_TAB, SDL_SCANCODE_PAGEUP, SDL_SCANCODE_PAGEDOWN, SDL_SCANCODE_P, SDL_SCANCODE_RETURN }
};
bool conflict[LAYK_Count];

long KeyInput;
long dbinput;
long inputBusy;
long joystick_read;
long joystick_read_x;
long joystick_read_y;
long joystick_read_fire;
static long joy_fire;
short ammo_change_timer = 0;
char ammo_change_buf[12];

LPCSTR GetKeyboardButtonNameFromScancode(SDL_Scancode code)
{
	// Avoid unknown id which is 0 !
	for (int i = 1; i < _countof(KeyboardButtons); i++)
	{
		const auto& key = KeyboardButtons[i];
		if (key.code == code)
			return key.keyName;
	}
	return KeyboardButtons[0].keyName; // return Unknown !
}

static void DoWeaponHotkey()
{
	short state;
	bool goin;

	if (!LaraItem)
		goin = 0;
	else
	{
		state = LaraItem->current_anim_state;
		goin = !(gfLevelFlags & GF_YOUNGLARA) && (Lara.waterStatus == LW_ABOVE_WATER || Lara.waterStatus == LW_WADE) && !bDisableLaraControl &&
			(state != AS_ALL4S && state != AS_CRAWL && state != AS_ALL4TURNL && state != AS_ALL4TURNR && state != AS_CRAWLBACK &&
				state != AS_CRAWL2HANG && state != AS_DUCK && state != AS_DUCKROTL && state != AS_DUCKROTR);
	}

	if (!goin)
		return;
	
	if (keymap[SDL_SCANCODE_1])
	{
		if (gfCurrentLevel < LVL5_THIRTEENTH_FLOOR)
		{
			if (!(Lara.pistolsTypeCarried & W_PRESENT))
				return;

			Lara.requestGunType = WEAPON_PISTOLS;

			if (Lara.gunStatus == LG_NO_ARMS && Lara.gunType == WEAPON_PISTOLS)
				Lara.gunStatus = LG_DRAW_GUNS;
		}
		else
		{
			if (!(Lara.hkTypeCarried & W_PRESENT))
				return;

			Lara.requestGunType = WEAPON_HK;

			if (Lara.gunType == WEAPON_HK)
			{
				if (Lara.gunStatus == LG_NO_ARMS)
					Lara.gunStatus = LG_DRAW_GUNS;
				else if (Lara.gunStatus == LG_READY && !ammo_change_timer)
				{
					if (!tomb5.ammotype_hotkeys)
						return;

					memset(ammo_change_buf, 0, sizeof(ammo_change_buf));

					if (Lara.hkTypeCarried & W_AMMO3)
					{
						Lara.hkTypeCarried &= ~W_AMMO3;
						Lara.hkTypeCarried |= W_AMMO2;
						ammo_change_timer = 30;
						sprintf(ammo_change_buf, "Burst");
					}
					else if (Lara.hkTypeCarried & W_AMMO2)
					{
						Lara.hkTypeCarried &= ~W_AMMO2;
						Lara.hkTypeCarried |= W_AMMO1;
						ammo_change_timer = 30;
						sprintf(ammo_change_buf, "Sniper");
					}
					else if (Lara.hkTypeCarried & W_AMMO1)
					{
						Lara.hkTypeCarried &= ~W_AMMO1;
						Lara.hkTypeCarried |= W_AMMO3;
						ammo_change_timer = 30;
						sprintf(ammo_change_buf, "Rapid");
					}
				}
			}
		}
	}
	else if (keymap[SDL_SCANCODE_2])
	{
		if (gfCurrentLevel < LVL5_THIRTEENTH_FLOOR)
		{
			if (!(Lara.shotgunTypeCarried & W_PRESENT))
				return;

			Lara.requestGunType = WEAPON_SHOTGUN;

			if (Lara.gunType == WEAPON_SHOTGUN)
			{
				if (Lara.gunStatus == LG_NO_ARMS)
					Lara.gunStatus = LG_DRAW_GUNS;
				else if (Lara.gunStatus == LG_READY && !ammo_change_timer)
				{
					if (!tomb5.ammotype_hotkeys)
						return;

					memset(ammo_change_buf, 0, sizeof(ammo_change_buf));

					if (Lara.shotgunTypeCarried & W_AMMO2)
					{
						Lara.shotgunTypeCarried &= ~W_AMMO2;
						Lara.shotgunTypeCarried |= W_AMMO1;
						ammo_change_timer = 30;
						sprintf(ammo_change_buf, "Normal");
					}
					else if (Lara.shotgunTypeCarried & W_AMMO1)
					{
						Lara.shotgunTypeCarried &= ~W_AMMO1;
						Lara.shotgunTypeCarried |= W_AMMO2;
						ammo_change_timer = 30;
						sprintf(ammo_change_buf, "Wideshot");
					}
				}
			}
		}
		else
		{
			if (!(Lara.crossbowTypeCarried & W_PRESENT))
				return;

			Lara.requestGunType = WEAPON_CROSSBOW;

			if (Lara.gunStatus == LG_NO_ARMS && Lara.gunType == WEAPON_CROSSBOW)
				Lara.gunStatus = LG_DRAW_GUNS;
		}
	}
	else if (keymap[SDL_SCANCODE_3])
	{
		if (!(Lara.uzisTypeCarried & W_PRESENT))
			return;

		Lara.requestGunType = WEAPON_UZI;

		if (Lara.gunStatus == LG_NO_ARMS && Lara.gunType == WEAPON_UZI)
			Lara.gunStatus = LG_DRAW_GUNS;
	}
	else if (keymap[SDL_SCANCODE_4])
	{
		if (!(Lara.sixshooterTypeCarried & W_PRESENT))
			return;

		Lara.requestGunType = WEAPON_REVOLVER;

		if (Lara.gunStatus == LG_NO_ARMS && Lara.gunType == WEAPON_REVOLVER)
			Lara.gunStatus = LG_DRAW_GUNS;
	}
}

bool Key(LayoutKeyType keyType)
{
	SDL_Scancode key = layout[LAY_Player][keyType];
	if (keymap[key] != 0) return true;

	if (conflict[keyType]) return false;

	key = layout[LAY_Default][keyType];
	return keymap[key] != 0;
}

static long KeyJoystick(long number)
{
	return 0;
}

long S_UpdateInput()
{
	static long linput;
	static long LookCnt;
	static long med_hotkey_timer;
	static long cheat_code;
	static long weird = 0;
	static long joy_x, joy_y;
	long flag;
	short state;
	static bool flare_no_db = 0;
	bool debounce;

	flag = 0;
	debounce = SetDebounce;

	DXUpdateJoystick();
	if (ControlMethod == CMT_Joystick)
		joy_fire = ReadJoystick(joy_x, joy_y);

	linput = 0;
	if (ControlMethod == CMT_Joystick)
	{
		if (joy_x < -8)
			linput = IN_LEFT;
		else if (joy_x > 8)
			linput = IN_RIGHT;
		if (joy_y > 8)
			linput |= IN_BACK;
		else if (joy_y < -8)
			linput |= IN_FORWARD;
	}

	if (Key(LAYK_Forward))
		linput |= IN_FORWARD;

	if (Key(LAYK_Backward))
		linput |= IN_BACK;

	if (Key(LAYK_Left))
		linput |= IN_LEFT;

	if (Key(LAYK_Right))
		linput |= IN_RIGHT;

	if (Key(LAYK_Duck))
		linput |= IN_DUCK;

	if (Key(LAYK_Sprint))
		linput |= IN_SPRINT;

	if (Key(LAYK_Walk))
		linput |= IN_WALK;

	if (Key(LAYK_Jump))
		linput |= IN_JUMP;

	if (Key(LAYK_Action))
		linput |= IN_SELECT | IN_ACTION;

	if (Key(LAYK_DrawWeapon))
		linput |= IN_DRAW;

	if (ControlMethod == CMT_Joystick)
	{
		// TODO: Finish joystick control method !

		if (KeyJoystick(jLayout[LAYK_JOY_Jump]))
			linput |= IN_JUMP;

		if (KeyJoystick(jLayout[LAYK_JOY_Action]))
			linput |= IN_SELECT | IN_ACTION;

		if (KeyJoystick(jLayout[LAYK_JOY_DrawWeapon]))
			linput |= IN_DRAW;

		if (KeyJoystick(jLayout[LAYK_JOY_Sprint]))
			linput |= IN_SPRINT;

		if (KeyJoystick(jLayout[LAYK_JOY_Walk]))
			linput |= IN_WALK;

		if (KeyJoystick(jLayout[LAYK_JOY_Duck]))
			linput |= IN_DUCK;

		if (KeyJoystick(jLayout[LAYK_JOY_UseFlare]))
			flag = 1;
	}

	if (Key(LAYK_UseFlare) || flag)
	{
		if (!flare_no_db)
		{
			state = LaraItem->current_anim_state;

			if (state == AS_ALL4S || state == AS_CRAWL || state == AS_ALL4TURNL ||
				state == AS_ALL4TURNR || state == AS_CRAWLBACK || state == AS_CRAWL2HANG)
			{
				SoundEffect(SFX_LARA_NO, 0, SFX_ALWAYS);
				flare_no_db = 1;
			}
			else
			{
				flare_no_db = 0;
				linput |= IN_FLARE;
			}
		}
	}
	else
	{
		flare_no_db = 0;
	}

	if (Key(LAYK_Look))
		linput |= IN_LOOK;

	if (Key(LAYK_Roll))
		linput |= IN_ROLL;

	if (Key(LAYK_Inventory))
		linput |= IN_OPTION;

	if (Key(LAYK_StepLeft))
		linput |= IN_WALK | IN_LEFT;

	if (Key(LAYK_StepRight))
		linput |= IN_WALK | IN_RIGHT;

	if (Key(LAYK_Pause))
		linput |= IN_PAUSE;

	if (Key(LAYK_Action))
		linput |= IN_SELECT;

	if (ControlMethod == CMT_Joystick)
	{
		if (KeyJoystick(jLayout[LAYK_JOY_Look]))
			linput |= IN_LOOK;

		if (KeyJoystick(jLayout[LAYK_JOY_Roll]))
			linput |= IN_ROLL;

		if (KeyJoystick(jLayout[LAYK_JOY_Select]))
			linput |= IN_OPTION;
	}

	if (keymap[SDL_SCANCODE_ESCAPE])
		linput |= IN_DESELECT | IN_OPTION;

	if (Lara.gunStatus == LG_READY)
	{
		savegame.AutoTarget = (uchar)App.AutoTarget;

		if (linput & IN_LOOK)
		{
			if (LookCnt >= 6)
				LookCnt = 100;
			else
			{
				linput &= ~IN_LOOK;
				LookCnt++;
			}
		}
		else
		{
			if (LookCnt && LookCnt != 100)
				linput |= IN_TARGET;

			LookCnt = 0;
		}
	}

	DoWeaponHotkey();

	if (keymap[SDL_SCANCODE_0])
	{
		if (!med_hotkey_timer)
		{
			if (LaraItem->hit_points > 0 && LaraItem->hit_points < 1000 || Lara.poisoned)
			{
				if (Lara.numSmallMedipack)
				{
					if (Lara.numSmallMedipack != -1)
						Lara.numSmallMedipack--;

					Lara.dpoisoned = 0;
					LaraItem->hit_points += 500;
					SoundEffect(SFX_MENU_MEDI, 0, SFX_ALWAYS);
					savegame.Game.HealthUsed++;

					if (LaraItem->hit_points > 1000)
						LaraItem->hit_points = 1000;

					if (InventoryActive && !Lara.numSmallMedipack)
					{
						construct_object_list();
						setup_objectlist_startposition(INV_COMPASS_ITEM);
					}

					med_hotkey_timer = 15;
				}
			}
		}
	}
	else if (keymap[SDL_SCANCODE_9])
	{
		if (!med_hotkey_timer)
		{
			if (LaraItem->hit_points > 0 && LaraItem->hit_points < 1000 || Lara.poisoned)
			{
				if (Lara.numLargeMedipack)
				{
					if (Lara.numLargeMedipack != -1)
						Lara.numLargeMedipack--;

					Lara.dpoisoned = 0;
					LaraItem->hit_points = 1000;
					SoundEffect(SFX_MENU_MEDI, 0, SFX_ALWAYS);
					savegame.Game.HealthUsed++;
					med_hotkey_timer = 15;

					if (InventoryActive && !Lara.numLargeMedipack)
					{
						construct_object_list();

						if (Lara.numSmallMedipack)
							setup_objectlist_startposition(INV_SMALLMEDI_ITEM);
						else
							setup_objectlist_startposition(INV_COMPASS_ITEM);
					}
				}
			}
		}
	}
	else if (med_hotkey_timer)
		med_hotkey_timer--;

	if (keymap[SDL_SCANCODE_F10])
		linput |= IN_E;

	if (linput & IN_WALK && !(linput & (IN_FORWARD | IN_BACK)))
	{
		if (linput & IN_LEFT)
			linput = (linput & ~IN_LEFT) | IN_LSTEP;
		else if (linput & IN_RIGHT)
			linput = (linput & ~IN_RIGHT) | IN_RSTEP;
	}

	if (linput & IN_FORWARD && linput & IN_BACK)
		linput |= IN_ROLL;

	if (linput & IN_ROLL && BinocularRange)
		linput &= ~IN_ROLL;

	if ((linput & (IN_RIGHT | IN_LEFT)) == (IN_RIGHT | IN_LEFT))
		linput -= IN_RIGHT | IN_LEFT;

	if (debounce)
		dbinput = inputBusy;

	if (!gfGameMode && Gameflow->LoadSaveEnabled)
	{
		if (keymap[SDL_SCANCODE_F5])
			linput |= IN_SAVE;

		if (keymap[SDL_SCANCODE_F6])
			linput |= IN_LOAD;
	}

	if (Gameflow->CheatEnabled)
	{
		if (linput)
			cheat_code = 0;

		switch (cheat_code)
		{
			case 0:

				if (keymap[SDL_SCANCODE_D])
					cheat_code = 1;

				break;

			case 1:

				if (keymap[SDL_SCANCODE_O])
					cheat_code = 2;

				break;

			case 2:

				if (keymap[SDL_SCANCODE_Z])
					cheat_code = 3;

				break;

			case 3:

				if (keymap[SDL_SCANCODE_Y])
					linput = IN_CHEAT;

				break;
		}
	}

	inputBusy = linput;

	if (Lara.busy)
	{
		linput &= IN_PAUSE | IN_LOOK | IN_OPTION | IN_RIGHT | IN_LEFT | IN_BACK | IN_FORWARD;
		if (linput & IN_FORWARD && linput & IN_BACK)
			linput ^= IN_BACK;
	}

	if (debounce)
		dbinput = inputBusy & (dbinput ^ inputBusy);

	KeyInput = linput;
	linput = weird;

	if (keymap[SDL_SCANCODE_F])
		linput = 1;

	weird &= weird ^ linput;
	return 1;
}

long ReadJoystick(long& x, long& y)
{
	if (joystick_read)
	{
		x = joystick_read_x;
		y = joystick_read_y;
		return joystick_read_fire;
	}

	x = 0;
	y = 0;
	return 0;
}
