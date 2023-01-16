#pragma once

#include <d3d11.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <mutex>
#include <condition_variable>

namespace Gdiplus {
	class Bitmap;
}

class GraphicsCapture
{
public:
	//GraphicsCapture() {}
	GraphicsCapture(HWND hWnd);
	~GraphicsCapture();

	HWND GetTarget() const { return hTargetWnd; }
	Gdiplus::Bitmap* ScreenShot();

private:
	void OnFrameArrived(const winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool& sender, const winrt::Windows::Foundation::IInspectable& args);
	winrt::com_ptr< ID3D11Texture2D> GetFrame();

public:
	static bool IsBorderRequiredSupported();
	static bool IsCursorCaptureEnabledSupported();
	static bool IsSupported();

private:
	HWND hTargetWnd = NULL;

	winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice d3ddevice;
	winrt::Windows::Graphics::Capture::GraphicsCaptureItem GraphicsCaptureItem = nullptr;
	winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool CaptureFramePool = nullptr;
	winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::FrameArrived_revoker FrameArrivedRevoker;
	winrt::Windows::Graphics::Capture::GraphicsCaptureSession GraphicsCaptureSession = nullptr;
	winrt::Windows::Graphics::SizeInt32 size;

	winrt::com_ptr<ID3D11Device> device;
	winrt::com_ptr<ID3D11DeviceContext> context;
	winrt::com_ptr<ID3D11Texture2D> texture;

	std::mutex mutex;
	std::condition_variable cond;
	bool hasData = false;
};

