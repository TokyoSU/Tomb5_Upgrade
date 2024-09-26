#pragma once
#include "../global/types.h"

void ClearSurfaces();
bool WinRunCheck(LPCSTR WindowName, LPCSTR ClassName, HANDLE* mutex);
float WinFrameRate();
void WinProcMsg();
void WinProcessCommands(long cmd);
void CLSetup(char* cmd);
void CLNoFMV(char* cmd);
void WinProcessCommandLine(LPSTR cmd);
//LRESULT CALLBACK WinMainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void WinClose();
bool WinRegisterWindow(HINSTANCE hinstance);
bool WinCreateWindow();
void WinSetStyle(bool fullscreen, ulong& set);

extern WINAPP App;
extern long resChangeCounter;
