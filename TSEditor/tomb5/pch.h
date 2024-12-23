// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#define WIN32_LEAN_AND_MEAN 
#define _USE_MATH_DEFINES
#define DIRECTINPUT_VERSION 0x0800

#include <windows.h>
#include <windowsx.h>
#include <stdint.h>
#include <stdio.h>
#include <mmeapi.h>
#include <d3d.h>
#include <d3dtypes.h>
#include <process.h>
#include <dinput.h>
#include <dsound.h>
#include <cmath>
#include <time.h>
#include <mmreg.h>
#include <mmiscapi.h>
#include <msacm.h>
#include <typeinfo>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include "resource.h"

#endif //PCH_H
