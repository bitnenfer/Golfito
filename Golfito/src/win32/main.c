#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#include <Windows.h>
#include <windowsx.h>
#include <dxgi.h>
#include <d3d11.h>

#include "../game/boot.h"
#include "../core/gfx.h"
#include "../core/input.h"
#include "../config/config_gfx.h"

extern void _input_win32_poll(void);
extern BOOL _input_win32_close_window(void);
extern void _gfx_d3d11_initialize(HWND windowHandle);
extern void _gfx_d3d11_swap_buffers(void);

static LRESULT CALLBACK WindowProc(HWND a, UINT b, WPARAM c, LPARAM d)
{
	switch (b)
	{
	case WM_CREATE:
		return 0;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	case WM_DESTROY:
		return 0;
	case WM_KEYDOWN:
		switch (c)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			return 0;
		}
	default:
		return DefWindowProc(a, b, c, d);
	}
}

int main() {
	// Create Window
	DWORD style = WS_VISIBLE | WS_CAPTION | WS_SYSMENU;
	RECT fullsize = { 0, 0, (LONG)GFX_DISPLAY_WIDTH, (LONG)GFX_DISPLAY_HEIGHT };
	HMODULE program = GetModuleHandle(NULL);
	WNDCLASS windowClass = { 0 };

	windowClass.style = CS_OWNDC;
	windowClass.lpfnWndProc = &WindowProc;
	windowClass.hInstance = program;
	windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = "WindowClass0";

	RegisterClass(&windowClass);
	AdjustWindowRect(&fullsize, style, 0);

	HWND windowHandle = CreateWindow("WindowClass0", GFX_WINDOW_TITLE, style, CW_USEDEFAULT, CW_USEDEFAULT, fullsize.right - fullsize.left, fullsize.bottom - fullsize.top, NULL, NULL, program, NULL);

	_gfx_d3d11_initialize(windowHandle);
	gfx_initialize();
	input_initialize();
	game_start();

	while (!_input_win32_close_window()) {
		_input_win32_poll();
		gfx_begin();
		game_loop(0.16f);
		gfx_end();
	}


	CloseWindow(windowHandle);
	DestroyWindow(windowHandle);

	return 0;
}