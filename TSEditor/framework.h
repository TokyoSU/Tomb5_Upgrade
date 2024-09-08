#pragma once
#pragma comment(lib, "D3D9")
#define FMT_UNICODE 0
#define SDL_MAIN_HANDLED

#include <iostream>
#include <memory>
#include <string>
#include <dxsdk-d3dx/d3dx9.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include "logger.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_dx9.h"
