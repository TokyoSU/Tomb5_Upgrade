#pragma once

class CWindow
{
public:
	bool Initialize(HINSTANCE hinstance);
	void Release();

	bool Create(int width, int height, LPCSTR name, LPCSTR class_name);
	void Update();

	void MakeFullscreen();
	void MakeWindowed();

	void Show();
	void Hide();

	void Center();

	bool IsOpened() const;
	HWND GetHandle() const;
	bool CheckIfInstanceAlreadyRunning() const;
private:
	SDL_Window* m_window = NULL;
	LPCSTR m_window_name = NULL;
	LPCSTR m_window_class_name = NULL;
	HANDLE m_mutex = NULL;
	HWND m_handle = NULL;
	bool m_is_opened = false;

	void UpdateWindow();
	void UpdateFrameRate();
};

extern CWindow g_Window;
