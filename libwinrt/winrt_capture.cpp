#include "GraphicsCapture.h"

#include "winrt_capture.h"

void __stdcall winrt_init_apartment()
{
	winrt::init_apartment(winrt::apartment_type::single_threaded);
}

struct GraphicsCapture* __stdcall winrt_init_capture(HWND hWnd)
{
	GraphicsCapture* capture = new GraphicsCapture(hWnd);
	return capture;
}

void __stdcall free_winrt_capture(GraphicsCapture* capture)
{
	delete capture;
}
Gdiplus::Bitmap* __stdcall winrt_screenshot(struct GraphicsCapture* capture)
{
	if (!capture) return nullptr;

	return capture->ScreenShot();
}
HWND __stdcall winrt_capture_get_target(struct GraphicsCapture* capture)
{
	if (!capture) return NULL;

	return capture->GetTarget();
}