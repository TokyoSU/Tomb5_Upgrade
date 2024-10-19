#include "../tomb5/pch.h"
#include "winmain.h"
#include "dxshell.h"
#include "function_stubs.h"
#include "output.h"
#include "../game/text.h"
#include "lighting.h"
#include "function_table.h"
#include "d3dmatrix.h"
#include "3dmath.h"
#include "audio.h"
#include "registry.h"
#include "../game/gameflow.h"
#include "texture.h"
#include "dxsound.h"
#include "gamemain.h"
#include "file.h"
#include "fmv.h"
#include "window.h"
#include "setupdlg.h"
#include "configuration.h"

WINAPP App;
long resChangeCounter;
static bool closing = false;

static COMMANDLINES commandlines[] =
{
	{ "SETUP", 0, &CLSetup },
	{ "NOFMV", 0, &CLNoFMV }
};

void ClearSurfaces()
{
	D3DRECT r{};
	r.x1 = App.dx.rViewport.left;
	r.y1 = App.dx.rViewport.top;
	r.y2 = App.dx.rViewport.top + App.dx.rViewport.bottom;
	r.x2 = App.dx.rViewport.left + App.dx.rViewport.right;
	DXAttempt(App.dx.lpViewport->Clear2(1, &r, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0F, 0));
	S_DumpScreen();
}

void WinProcessCommands(long cmd)
{
	DXDISPLAYMODE* dm;
	long odm;

	if (cmd == KA_ALTENTER)
	{
		if (App.fmv || !(G_dxinfo->DDInfo[G_dxinfo->nDD].DDCaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED) || LevelLoadingThread.active)
			return;

		Log("KA_ALTENTER");

		Log("HangGameThread");
		while (App.dx.InScene) {};
		App.dx.WaitAtBeginScene = TRUE;
		while (!App.dx.InScene) {};

		//SuspendThread(MainThread.handle);
		Log("Game Thread Suspended");

		DXToggleFullScreen();
		HWInitialise();
		S_InitD3DMatrix();
		aSetViewMatrix();

		//ResumeThread(MainThread.handle);
		App.dx.WaitAtBeginScene = 0;
		Log("Game Thread Resumed");

		if (App.dx.Flags & DXF_FULLSCREEN)
		{
			SetCursor(0);
			ShowCursor(0);
		}
		else
		{
			SetCursor(LoadCursor(0, IDC_ARROW));
			ShowCursor(1);
		}
	}
	else if (cmd == KA_ALTP || cmd == KA_ALTM)
	{
		if (LevelLoadingThread.active || App.fmv)
			return;

		Log("Change Video Mode");

		Log("HangGameThread");
		while (App.dx.InScene) {};
		App.dx.WaitAtBeginScene = 1;
		while (!App.dx.InScene) {};
		//SuspendThread(MainThread.handle);
		Log("Game Thread Suspended");

		odm = App.DXInfo.nDisplayMode;

		if (cmd == KA_ALTP)
		{
			App.DXInfo.nDisplayMode++;

			if (App.DXInfo.nDisplayMode >= G_dxinfo->DDInfo[G_dxinfo->nDD].D3DDevices[G_dxinfo->nD3D].nDisplayModes)
				App.DXInfo.nDisplayMode = G_dxinfo->DDInfo[G_dxinfo->nDD].D3DDevices[G_dxinfo->nD3D].nDisplayModes - 1;

			dm = G_dxinfo->DDInfo[App.DXInfo.nDD].D3DDevices[App.DXInfo.nD3D].DisplayModes;

			while (dm[odm].bpp != dm[App.DXInfo.nDisplayMode].bpp)
			{
				App.DXInfo.nDisplayMode++;

				if (App.DXInfo.nDisplayMode >= G_dxinfo->DDInfo[G_dxinfo->nDD].D3DDevices[G_dxinfo->nD3D].nDisplayModes)
				{
					App.DXInfo.nDisplayMode = odm;
					break;
				}
			}
		}
		else
		{
			App.DXInfo.nDisplayMode--;

			if (App.DXInfo.nDisplayMode < 0)
				App.DXInfo.nDisplayMode = 0;

			dm = G_dxinfo->DDInfo[App.DXInfo.nDD].D3DDevices[App.DXInfo.nD3D].DisplayModes;

			while (dm[odm].bpp != dm[App.DXInfo.nDisplayMode].bpp)
			{
				App.DXInfo.nDisplayMode--;

				if (App.DXInfo.nDisplayMode < 0)
				{
					App.DXInfo.nDisplayMode = odm;
					break;
				}
			}
		}

		if (odm != App.DXInfo.nDisplayMode)
		{
			if (!DXChangeVideoMode())
			{
				App.DXInfo.nDisplayMode = odm;
				DXChangeVideoMode();
			}

			HWInitialise();
			InitWindow(0, 0, App.dx.dwRenderWidth, App.dx.dwRenderHeight, 20, 20480, 80, App.dx.dwRenderWidth, App.dx.dwRenderHeight);
			InitFont();
			S_InitD3DMatrix();
			aSetViewMatrix();
		}

		//ResumeThread(MainThread.handle);
		App.dx.WaitAtBeginScene = 0;
		Log("Game Thread Resumed");
		resChangeCounter = 120;
	}
}

void CLSetup(char* cmd)
{
	Log(__FUNCTION__);
	start_setup = !(cmd != NULL);
}

void CLNoFMV(char* cmd)
{
	Log(__FUNCTION__);
	fmvs_disabled = !(cmd != NULL);
}

void WinProcessCommandLine(LPSTR cmd)
{
	COMMANDLINES* command;
	char* pCommand;
	char* p;
	char* last;
	ulong l;
	long num;
	char parameter[20];

	Log(__FUNCTION__);

	num = sizeof(commandlines) / sizeof(commandlines[0]);

	for (int i = 0; i < num; i++)
	{
		command = &commandlines[i];
		command->code((char*)"_INIT");
	}

	for (int i = 0; (ulong)i < strlen(cmd); i++)
	{
		if (toupper(cmd[i]))
			cmd[i] = toupper(cmd[i]);
	}

	for (int i = 0; i < num; i++)
	{
		command = &commandlines[i];
		memset(parameter, 0, sizeof(parameter));
		pCommand = strstr(cmd, command->command);

		if (pCommand)
		{
			if (command->needs_parameter)
			{
				p = 0;
				l = strlen(pCommand);

				for (int j = 0; (ulong)j < l; j++, pCommand++)
				{
					if (*pCommand != '=')
						continue;

					p = pCommand + 1;
					l = strlen(p);

					for (j = 0; (ulong)j < l; j++, p++)
					{
						if (*p != ' ')
							break;
					}

					last = p;
					l = strlen(last);

					for (j = 0; (ulong)j < l; j++, last++)
					{
						if (*last == ' ')
							break;
					}

					strncpy(parameter, p, j);
					break;
				}

				command->code(parameter);
			}
			else
				command->code(0);
		}
	}
}

void WinClose()
{
	Log(__FUNCTION__);

	SaveSettings();

	if (G_dxptr != NULL)
	{
		DXAttempt(G_dxptr->Keyboard->Unacquire());
		SafeRelease(G_dxptr->Keyboard, "Keyboard");
		SafeRelease(G_dxptr->lpDirectInput, "DirectInput");
	}

	DXFreeInfo(&App.DXInfo);
	DXClose();
	FreeBinkStuff();
	DestroyAcceleratorTable(App.hAccel);
	
	g_Window.Release();
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	DXDISPLAYMODE* dm;
	start_setup = false;
	App.mmx = false;
	App.SetupComplete = false;
	App.AutoTarget = FALSE;

	atexit(WinClose);

	g_Config.Initialize();
	g_Window.Initialize(hInstance);
	if (g_Window.CheckIfInstanceAlreadyRunning()) {
		exit(EXIT_SUCCESS);
	}

	LoadGameflow();
	WinProcessCommandLine(lpCmdLine);

	g_Window.Create(640, 480, "Tomb Raider Chronicles", "Tomb5:Class");

	DXGetInfo(&App.DXInfo, g_Window.GetHandle());

	if (start_setup || !LoadSettings())
	{
		if (!SetupDialog())
		{
			SafeFree(gfScriptFile);
			SafeFree(gfLanguageFile);
			exit(EXIT_SUCCESS);
		}
		LoadSettings();
	}

	g_Window.Show();

	if (!fmvs_disabled && !LoadBinkStuff())
	{
		Log("Failed to load Bink, disabling FMVs.");
		fmvs_disabled = true;
	}

	App.dx.WaitAtBeginScene = FALSE;
	App.dx.InScene = FALSE;
	App.fmv = false;

	dm = &G_dxinfo->DDInfo[G_dxinfo->nDD].D3DDevices[G_dxinfo->nD3D].DisplayModes[G_dxinfo->nDisplayMode];
	if (!DXCreate(dm->w, dm->h, dm->bpp, App.StartFlags, &App.dx, g_Window.GetHandle()))
	{
		MessageBox(0, SCRIPT_TEXT(TXT_Failed_To_Setup_DirectX), "Tomb Raider", 0);
		exit(EXIT_FAILURE);
	}

	if (G_dxptr->Flags & DXF_FULLSCREEN)
	{
		ShowCursor(FALSE);
		g_Window.MakeFullscreen();
	}
	else
	{
		ShowCursor(TRUE);
		g_Window.MakeWindowed();
	}

	DXInitInput(g_Window.GetHandle(), App.hInstance);
	App.hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
	if (!App.SoundDisabled)
	{
		DXDSCreate();
		ACMInit();
	}

	GameMain(NULL);

	exit(EXIT_SUCCESS);
}
