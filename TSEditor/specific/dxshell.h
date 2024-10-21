#pragma once
#include "../global/types.h"

void DXBitMask2ShiftCnt(ulong mask, uchar* shift, uchar* count);
long DXAttempt(HRESULT r);
long DIAttempt(HRESULT r);
void* AddStruct(void* p, long num, long size);
long DXDDCreate(LPGUID pGuid, void** pDD4);
long DXD3DCreate(LPDIRECTDRAW7 pDD4, void** pD3D);
long DXSetCooperativeLevel(LPDIRECTDRAW7 pDD4, HWND hwnd, long flags);
HRESULT CALLBACK DXEnumDisplayModes(DDSURFACEDESC2* lpDDSurfaceDesc2, LPVOID lpContext);
HRESULT CALLBACK DXEnumZBufferFormats(LPDDPIXELFORMAT lpDDPixFmt, LPVOID lpContext);
HRESULT CALLBACK DXEnumTextureFormats(LPDDPIXELFORMAT lpDDPixFmt, LPVOID lpContext);
long BPPToDDBD(long BPP);
long DXCreateD3DDevice(LPDIRECT3D7 d3d, GUID guid, LPDIRECTDRAWSURFACE7 surf, LPDIRECT3DDEVICE7* device);
long DXSetVideoMode(LPDIRECTDRAW7 dd, long dwWidth, long dwHeight, long dwBPP);
long DXCreateSurface(LPDIRECTDRAW7 dd, LPDDSURFACEDESC2 desc, LPDIRECTDRAWSURFACE7* surf);
long DXCreateViewport(LPDIRECT3D7 d3d, LPDIRECT3DDEVICE7 device, long w, long h, LPDIRECT3DVIEWPORT3* viewport);
void DXSaveScreen(LPDIRECTDRAWSURFACE7 surf, const char* name);
HRESULT DXShowFrame();
void DXMove(long x, long y);
void DXClose();
long DXCreate(long w, long h, long bpp, long Flags, DXPTR* dxptr, HWND hWnd);
long DXChangeVideoMode();
long DXToggleFullScreen();
HRESULT CALLBACK DXEnumDirect3D(LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC7, LPVOID lpContext);
BOOL CALLBACK DXEnumDirectDraw(GUID FAR* lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext);
BOOL CALLBACK DXEnumDirectSound(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext);
long DXGetInfo(DXINFO* dxinfo, HWND hwnd);
void DXFreeInfo(DXINFO* dxinfo);
long DXUpdateJoystick();
void DXInitInput(HWND hwnd, HINSTANCE hinstance);
const char* DXGetErrorString(HRESULT hr);
const char* DIGetErrorString(HRESULT hr);

extern DXPTR* G_dxptr;
extern DXINFO* G_dxinfo;
extern LPDIRECTDRAW7 G_ddraw;
extern LPDIRECT3D7 G_d3d;
extern HWND G_hwnd;
extern Uint8 keymap[SDL_NUM_SCANCODES];
