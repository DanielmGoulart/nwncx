#pragma once
#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <stdlib.h>

class WindowApp
{
public:
	inline WindowApp() :
		hwnd_(NULL),
		window_width_(0),
		window_height_(0)
	{};
	~WindowApp();

	bool Initialize(
		HINSTANCE hInstance,
		int nCmdShow,
		int x = CW_USEDEFAULT,
		int y = CW_USEDEFAULT,
		int nWidth = CW_USEDEFAULT,
		int nHeight = CW_USEDEFAULT);

	bool ProcessWindowMessage();
	void CloseWindow();

private:
	HWND hwnd_;
	int window_width_;
	int window_height_;

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

