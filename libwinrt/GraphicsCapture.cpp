#include "GraphicsCapture.h"

#include <inspectable.h>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/windows.foundation.metadata.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <Windows.Graphics.Capture.Interop.h>
#include <dxgi.h>

#include <Windows.h>
#include <gdiplus.h>
#include <dwmapi.h>

GraphicsCapture::GraphicsCapture(HWND hWnd) : hTargetWnd(hWnd)
{
	D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, D3D11_SDK_VERSION, device.put(), nullptr, context.put());

	winrt::com_ptr<IDXGIDevice> DxgiDevice;
	auto hr = device->QueryInterface<IDXGIDevice>(DxgiDevice.put());

	winrt::com_ptr<IInspectable> gdevice;
	CreateDirect3D11DeviceFromDXGIDevice(DxgiDevice.get(), gdevice.put());
	this->d3ddevice = gdevice.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();

	auto interop = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>().as<IGraphicsCaptureItemInterop>();
	interop->CreateForWindow(hWnd, winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), winrt::put_abi(GraphicsCaptureItem));

	this->size = GraphicsCaptureItem.Size();

	CaptureFramePool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::Create(this->d3ddevice, winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized, 1, size);

	//FrameArrivedRevoker = CaptureFramePool.FrameArrived(winrt::auto_revoke, { this, &GraphicsCapture::OnFrameArrived });
	GraphicsCaptureSession = CaptureFramePool.CreateCaptureSession(GraphicsCaptureItem);
	if (IsBorderRequiredSupported()) GraphicsCaptureSession.IsBorderRequired(false);
	if (IsCursorCaptureEnabledSupported()) GraphicsCaptureSession.IsCursorCaptureEnabled(false);

	GraphicsCaptureSession.StartCapture();
}

GraphicsCapture::~GraphicsCapture()
{
	//FrameArrivedRevoker.revoke();

	GraphicsCaptureSession.Close();
	GraphicsCaptureSession = nullptr;

	CaptureFramePool.Close();
	CaptureFramePool = nullptr;

	context = nullptr;
	d3ddevice = nullptr;
	GraphicsCaptureItem = nullptr;
}

Gdiplus::Bitmap* GraphicsCapture::ScreenShot()
{
	D3D11_TEXTURE2D_DESC desc, imgdesc;
	RECT rc, rw;

	GetClientRect(hTargetWnd, &rc);
	DwmGetWindowAttribute(hTargetWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rw, sizeof(rw));

	if (rc.right == 0 || rc.bottom == 0) return nullptr;

	auto texture = GetFrame();
	if (!texture) return nullptr;

	texture->GetDesc(&desc);

	imgdesc.ArraySize = 1;
	imgdesc.BindFlags = 0;
	imgdesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	imgdesc.Format = desc.Format;
	imgdesc.Width = desc.Width;
	imgdesc.Height = desc.Height;
	imgdesc.MipLevels = 1;
	imgdesc.MiscFlags = 0;
	imgdesc.SampleDesc.Count = 1;
	imgdesc.SampleDesc.Quality = 0;
	imgdesc.Usage = D3D11_USAGE_STAGING;

	ID3D11Texture2D* pCPUTexture;
	D3D11_MAPPED_SUBRESOURCE subresource;

	winrt::com_ptr<ID3D11DeviceContext> context;
	device->GetImmediateContext(context.put());

	device->CreateTexture2D(&imgdesc, 0, &pCPUTexture);
	context->CopyResource(pCPUTexture, texture.get());
	context->Map(pCPUTexture, 0, D3D11_MAP_READ, 0, &subresource);

	Gdiplus::Bitmap* raw = new Gdiplus::Bitmap(desc.Width, desc.Height, subresource.RowPitch, PixelFormat32bppRGB, (byte*)subresource.pData);

	int x = (rw.right - rw.left - rc.right) / 2;
	int y = (rw.bottom - rw.top - rc.bottom - x * 2) + x;
	int width = rc.right;
	int height = rc.bottom;

	Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(width, height, PixelFormat24bppRGB);
	{
		Gdiplus::Graphics g(bitmap);
		g.DrawImage(raw, Gdiplus::Point(-x, -y));
	}

	delete raw;

	context->Unmap(pCPUTexture, 0);

	return bitmap;
}

void GraphicsCapture::OnFrameArrived(const winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool& sender, const winrt::Windows::Foundation::IInspectable& args)
{
	hasData = true;
	cond.notify_all();
}

winrt::com_ptr<ID3D11Texture2D> GraphicsCapture::GetFrame()
{
	bool skip = true;

	for (int c = 0; ; c++) {
		//std::call_once(once, [&] { return CaptureFramePool.TryGetNextFrame(); });
		auto frame = CaptureFramePool.TryGetNextFrame();
		if (!frame) {
			continue;
		}

		if (skip) {
			skip = false;
			continue;
		}

		auto surface = frame.Surface();
		if (!surface) return nullptr;

		winrt::com_ptr<ID3D11Texture2D> texture;

		auto access = surface.as<Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();
		HRESULT hr = access->GetInterface(winrt::guid_of<ID3D11Texture2D>(), texture.put_void());
		if (FAILED(hr)) return nullptr;

		auto size = frame.ContentSize();

		if (size.Width != this->size.Width || size.Height != this->size.Height) {
			this->size = size;
			CaptureFramePool.Recreate(d3ddevice, winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized, 1, size);
		}

		return texture;
	}

	return nullptr;
}

bool GraphicsCapture::IsBorderRequiredSupported()
{
	try {
		return winrt::Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent(L"Windows.Graphics.Capture.GraphicsCaptureSession", L"IsBorderRequired");
	}
	catch (...) {
		return false;
	}
}

bool GraphicsCapture::IsCursorCaptureEnabledSupported()
{
	try {
		return winrt::Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent(L"Windows.Graphics.Capture.GraphicsCaptureSession", L"IsCursorCaptureEnabled");
	}
	catch (...) {
		return false;
	}
}

bool GraphicsCapture::IsSupported()
{
	try {
		return winrt::Windows::Foundation::Metadata::ApiInformation::IsTypePresent(L"Windows.Graphics.Capture.GraphicsCaptureSession") &&
			winrt::Windows::Graphics::Capture::GraphicsCaptureSession::IsSupported();
	}
	catch (...) {
		return false;
	}
}
