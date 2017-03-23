#include "window.h"

WindowApp::~WindowApp() {

}

bool WindowApp::Initialize(HINSTANCE hInstance, int nCmdShow, int x, int y, int nWidth, int nHeight) {
	LPTSTR pWndClass = "Window";

	WNDCLASS wc = {};
	wc.hInstance = hInstance;
	wc.lpfnWndProc = this->WindowProc;
	wc.lpszClassName = pWndClass;

	ATOM wndClass = RegisterClass(&wc);
	if (!wndClass)
		return false;

	hwnd_ = CreateWindowEx(
		0,
		pWndClass,
		TEXT("NWNCX Loader"),
		WS_OVERLAPPEDWINDOW,
		x, y, nWidth, nHeight,
		NULL,
		NULL,
		hInstance,
		this
		);

	if (!hwnd_) {
		DWORD dwError = GetLastError();
		return false;
	}

	ShowWindow(hwnd_, nCmdShow);
	UpdateWindow(hwnd_);

	RECT rc;
	if (!GetClientRect(hwnd_, &rc))
		return false;

	window_width_ = abs(rc.right - rc.left);
	window_height_ = abs(rc.bottom - rc.top);
	
	return true;

}

bool WindowApp::ProcessWindowMessage() {
	MSG msg;

	if (GetMessage(&msg, NULL, 0, 0))
	{

		TranslateMessage(&msg);
		DispatchMessage(&msg);

	}
	if (msg.message == WM_QUIT)
		return false;

	return true;
}

void WindowApp::CloseWindow() {
	PostQuitMessage(0);
}

LRESULT CALLBACK WindowApp::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_CREATE) {
		WindowApp *pThis = (WindowApp*)(((CREATESTRUCT*)lParam)->lpCreateParams);
		if (!pThis) {
			PostQuitMessage(0);
			return 0;
		}
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
		return 0;

	}
	else {
		WindowApp *pThis = (WindowApp*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		switch (uMsg)
		{
		case WM_PAINT:
		{
			return 0;
		}
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}