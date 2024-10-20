#include "../tomb5/pch.h"
#include "window.h"
#include "game/gameflow.h"
#include "dxshell.h"
#include "gamemain.h"
#include "function_stubs.h"
#include "winmain.h"
#include "input.h"

CWindow g_Window;

bool CWindow::Initialize(HINSTANCE hinstance)
{
	App.hInstance = hinstance;
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
	{
		Log("Failed to initialize SDL2, Error: %s", SDL_GetError());
		return false;
	}
	return true;
}

void CWindow::Release()
{
	CloseHandle(m_mutex);
	if (m_window)
	{
		SDL_DestroyWindow(m_window);
		m_window = NULL;
	}
	SDL_Quit();
}

bool CWindow::Create(int width, int height, LPCSTR name, LPCSTR class_name)
{
    m_mutex = CreateMutex(NULL, TRUE, name);

    m_window_name = name;
    m_window_class_name = class_name;
	m_window = SDL_CreateWindow(name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_HIDDEN);
	if (m_window == NULL)
	{
		Log("Failed to create SDL window, Error: %s", SDL_GetError());
		return false;
	}

	SDL_SysWMinfo data;
	ZeroMemory(&data, sizeof(data));
	SDL_VERSION(&data.version);
	if (SDL_GetWindowWMInfo(m_window, &data) == SDL_FALSE)
	{
		Log("Failed to get the window SDL handle, Error: %s", SDL_GetError());
		return false;
	}

	m_handle = data.info.win.window;
	m_is_opened = m_handle != NULL;
	return m_is_opened;
}

void CWindow::Update()
{
	UpdateWindow();
	UpdateFrameRate();
}

void CWindow::MakeFullscreen()
{
	SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);
}

void CWindow::MakeWindowed()
{
	SDL_SetWindowFullscreen(m_window, NULL);
}

void CWindow::Show()
{
	SDL_ShowWindow(m_window);
}

void CWindow::Hide()
{
	SDL_HideWindow(m_window);
}

void CWindow::Center()
{
	SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void CWindow::Resize(int width, int height)
{
	SDL_SetWindowSize(m_window, width, height);
}

bool CWindow::IsOpened() const
{
    return m_is_opened;
}

HWND CWindow::GetHandle() const
{
    return m_handle;
}

bool CWindow::CheckIfInstanceAlreadyRunning() const
{
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        auto window = FindWindow(m_window_class_name, m_window_name);
        if (window)
            MessageBox(NULL, "Failed to launch the game, there is already a instance of it !", m_window_name, MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST);
        return true;
    }
    return false;
}

void CWindow::UpdateWindow()
{
	SDL_Event evts;
	while (SDL_PollEvent(&evts) != 0)
	{
		switch (evts.type)
		{
		case SDL_QUIT:
		case SDL_APP_TERMINATING:
			m_is_opened = false;
			break;
		case SDL_KEYDOWN:
			keymap[evts.key.keysym.scancode] = 1;
			break;
		case SDL_KEYUP:
			keymap[evts.key.keysym.scancode] = 0;
			break;
		case SDL_WINDOWEVENT:
			switch (evts.window.event)
			{
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				Log("SDL_WINDOWEVENT_FOCUS_GAINED");
				if (App.SetupComplete)
				{
					App.dx.WaitAtBeginScene = FALSE;
					Log("Game Thread Resumed");
				}
				break;
			case SDL_WINDOWEVENT_FOCUS_LOST:
				Log("SDL_WINDOWEVENT_FOCUS_LOST");
				if (App.SetupComplete)
				{
					App.dx.WaitAtBeginScene = TRUE;
					Log("Game Thread Suspended");
				}
				break;
			case SDL_WINDOWEVENT_MOVED:
				Log("SDL_WINDOWEVENT_MOVED");
				DXMove(evts.window.data1, evts.window.data2);
				break;
			}
			break;
		}
	}
}

void CWindow::UpdateFrameRate()
{
	double t, time_now;
	static float fps;
	static long time, counter;
	static char first_time;

	if (!(first_time & 1))
	{
		first_time |= 1;
		time = clock();
	}

	counter++;
	if (counter == 10)
	{
		time_now = clock();
		t = (time_now - time) / (double)CLOCKS_PER_SEC;
		time = (long)time_now;
		fps = float(counter / t);
		counter = 0;
	}

	App.fps = fps;
}
