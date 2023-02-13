#include "GraphicsCapture.h"

#include "winrt_capture.h"

EXPORT void __stdcall winrt_init_apartment()
{
	winrt::init_apartment(winrt::apartment_type::single_threaded);
}

EXPORT struct GraphicsCapture* __stdcall winrt_init_capture(HWND hWnd)
{
	GraphicsCapture* capture = nullptr;

	try {
		capture = new GraphicsCapture(hWnd);
	}
	catch (...) {
		if (capture) delete capture;
		return nullptr;
	}

	return capture;
}

EXPORT void __stdcall free_winrt_capture(GraphicsCapture* capture)
{
	delete capture;
}

EXPORT Gdiplus::Bitmap* __stdcall winrt_screenshot(struct GraphicsCapture* capture)
{
	if (!capture) return nullptr;

	return capture->ScreenShot();
}

EXPORT HWND __stdcall winrt_capture_get_target(struct GraphicsCapture* capture)
{
	if (!capture) return NULL;

	return capture->GetTarget();
}

EXPORT bool winrt_capture_is_supported()
{
	return GraphicsCapture::IsSupported();
}
