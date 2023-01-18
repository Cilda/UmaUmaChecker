#pragma once

#ifdef __EXPORT
#define EXPORT extern "C" __declspec(dllexport)
#include <gdiplus.h>
#else
#define EXPORT extern "C" __declspec(dllimport)
namespace Gdiplus {
	class Bitmap;
}
#endif



EXPORT void __stdcall winrt_init_apartment();
EXPORT struct GraphicsCapture* __stdcall winrt_init_capture(HWND hWnd);
EXPORT void __stdcall free_winrt_capture(struct GraphicsCapture* capture);
EXPORT Gdiplus::Bitmap* winrt_screenshot(struct GraphicsCapture* capture);
EXPORT HWND winrt_capture_get_target(struct GraphicsCapture* capture);
