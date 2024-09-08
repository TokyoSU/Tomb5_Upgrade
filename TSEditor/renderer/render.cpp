#include "framework.h"
#include "render.h"

CRender Render;

void CRender::Initialize(HWND hWnd, int width, int height, bool isFullscreen)
{
	// Release previous created D3D.
	Release3D();
	Release3DDevice();

	// Now create again.
	_D3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (_D3D == NULL)
	{
		DrawMessageBox(MsgType::Error, "D3D Error", "Failed to create D3D !");
		exit(EXIT_FAILURE);
	}

	ZeroMemory(&_D3DParam, sizeof(_D3DParam));
	_D3DParam.Windowed = TRUE;
	_D3DParam.BackBufferCount = 1;
	_D3DParam.BackBufferWidth = width;
	_D3DParam.BackBufferHeight = height;
	_D3DParam.BackBufferFormat = D3DFMT_A8R8G8B8;
	_D3DParam.hDeviceWindow = hWnd;
	_D3DParam.EnableAutoDepthStencil = TRUE;
	_D3DParam.AutoDepthStencilFormat = D3DFMT_D16;
	_D3DParam.SwapEffect = D3DSWAPEFFECT_DISCARD;
	_D3DParam.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // Vsync on
	_D3DParam.MultiSampleType = D3DMULTISAMPLE_NONE;
	_D3DParam.MultiSampleQuality = 0;
	_D3DParam.FullScreen_RefreshRateInHz = 0; // Need to be 0 for windowed, else refresh rate for fullscreen !
	if FAILED(_D3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &_D3DParam, &_D3DDevice))
	{
		DrawMessageBox(MsgType::Error, "D3D Error", "Failed to create D3DDevice !");
		exit(EXIT_FAILURE);
	}

	if (!ImGui_ImplDX9_Init(_D3DDevice))
	{
		DrawMessageBox(MsgType::Error, "ImGui Error", "Failed to init imgui dx9 implementation !");
		exit(EXIT_FAILURE);
	}
}

void CRender::Release3D()
{
	if (_D3D != NULL)
	{
		_D3D->Release();
		_D3D = NULL;
	}
}

void CRender::Release3DDevice()
{
	if (_D3DDevice != NULL)
	{
		_D3DDevice->EvictManagedResources();
		_D3DDevice->Release();
		_D3DDevice = NULL;
	}
}

void CRender::Release3DImGui()
{
	ImGui_ImplDX9_Shutdown();
}

void CRender::ClearScreen()
{
	if FAILED(_D3DDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0))
		TraceLog(LogType::Warn, "Failed to clear the screen through _D3DDevice->Clear !");
}

void CRender::BeginScene()
{
	if FAILED(_D3DDevice->BeginScene())
		TraceLog(LogType::Warn, "Failed to begin the scene through _D3DDevice->BeginScene !");
}

void CRender::EndScene()
{
	if FAILED(_D3DDevice->EndScene())
		TraceLog(LogType::Warn, "Failed to begin the scene through _D3DDevice->EndScene !");
}

void CRender::Present()
{
	HRESULT hr = _D3DDevice->Present(NULL, NULL, NULL, NULL);
	if FAILED(hr)
	{
		TraceLog(LogType::Warn, "Failed to draw on screen through _D3DDevice->Present !");
		if (hr == D3DERR_DEVICELOST)
			TraceLog(LogType::Warn, "- Device lost !");
	}
}

void CRender::SetZBuffer(bool enable)
{
	if (_ZBufferEnabled != enable)
	{
		if FAILED(_D3DDevice->SetRenderState(D3DRS_ZENABLE, enable ? D3DZB_TRUE : D3DZB_FALSE))
			TraceLog(LogType::Warn, "Failed to set zbuffer through _D3DDevice->SetRenderState(D3DRS_ZENABLE) !");
		_ZBufferEnabled = enable;
	}
}

void CRender::SetAlphaBlend(bool enable)
{
	if (_AlphaBlendEnable != enable)
	{
		if FAILED(_D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, enable ? TRUE : FALSE))
			TraceLog(LogType::Warn, "Failed to set alpha blend through _D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE) !");
		_AlphaBlendEnable = enable;
	}
}

void CRender::SetScissorTest(bool enable)
{
	if (_ScissorTestEnable != enable)
	{
		if FAILED(_D3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, enable ? TRUE : FALSE))
			TraceLog(LogType::Warn, "Failed to set scissor test through _D3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE) !");
		_ScissorTestEnable = enable;
	}
}
