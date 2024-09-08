#pragma once

class CRender
{
public:
	CRender() {}
	~CRender() {}

	void Initialize(HWND hWnd, int width, int height, bool isFullscreen);
	void Release3D();
	void Release3DDevice();
	void Release3DImGui();
	void ClearScreen();
	void BeginScene();
	void EndScene();
	void Present();

	void SetZBuffer(bool enable);
	void SetAlphaBlend(bool enable);
	void SetScissorTest(bool enable);

private:
	LPDIRECT3D9 _D3D = NULL;
	LPDIRECT3DDEVICE9 _D3DDevice = NULL;
	D3DPRESENT_PARAMETERS _D3DParam = {};


private:
	bool _ZBufferEnabled = false;
	bool _AlphaBlendEnable = false;
	bool _ScissorTestEnable = false;
};

extern CRender Render;
