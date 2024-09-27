#pragma once
#include "../global/types.h"

void ClearSurfaces();
void WinProcessCommands(long cmd);
void CLSetup(char* cmd);
void CLNoFMV(char* cmd);
void WinProcessCommandLine(LPSTR cmd);
void WinClose();

extern WINAPP App;
extern long resChangeCounter;
