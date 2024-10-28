#pragma once
#include "../global/types.h"

LPCSTR GetKeyboardButtonNameFromScancode(SDL_Scancode code);
bool Key(LayoutKeyType number);
long S_UpdateInput();
long ReadJoystick(long& x, long& y);

extern KeyButton KeyboardButtons[SDL_NUM_SCANCODES];
extern KeyButton GermanKeyboard[SDL_NUM_SCANCODES];
extern const char* JoyStickButtons[16];
extern long jLayout[32];
extern long defaultJLayout[32];

extern SDL_Scancode layout[LAY_Count][LAYK_Count];
extern bool conflict[LAYK_Count];
extern long KeyInput;
extern long dbinput;
extern long inputBusy;
extern long joystick_read;
extern long joystick_read_x;
extern long joystick_read_y;
extern long joystick_read_fire;
extern short ammo_change_timer;
extern char ammo_change_buf[12];
