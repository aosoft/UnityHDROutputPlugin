#include "stdafx.h"
#include "RenderTarget.h"

inline ComPtr<ID3D11RenderTargetView> CreateRenderTargetView(ID3D11Device *device, IDXGISwapChain *swapchain)
{
	ComPtr<ID3D11RenderTargetView> ret;
	ComPtr<ID3D11Texture2D> tex;

	HRException::CheckHR(swapchain->GetBuffer(0, IID_PPV_ARGS(&tex)));
	HRException::CheckHR(device->CreateRenderTargetView(tex, nullptr, &ret));

	return ret;
}

RenderTarget::RenderTarget(HWND hwnd, ComPtr<ID3D11Device> const& device) :
	_hwnd(hwnd),
	_device(device)
{
	ComPtr<IDXGIDevice> dxgidevice;
	ComPtr<IDXGIAdapter> adapter;
	RECT rect;

	HRException::CheckNull(device);

	HRException::CheckHR(device->QueryInterface(&dxgidevice));
	HRException::CheckHR(dxgidevice->GetParent(IID_PPV_ARGS(&adapter)));
	HRException::CheckHR(adapter->GetParent(IID_PPV_ARGS(&_factory)));
	if (FAILED(_factory->QueryInterface(&_factory2)))
	{
		_factory2 = nullptr;
	}

	::GetClientRect(hwnd, &rect);

	{
		auto desc = DXGI_SWAP_CHAIN_DESC();
		desc.BufferDesc.Width = rect.right - rect.left;
		desc.BufferDesc.Height = rect.bottom - rect.top;
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = 1;
		desc.Windowed = TRUE;
		desc.OutputWindow = hwnd;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		HRException::CheckHR(_factory->CreateSwapChain(device, &desc, &_swapchain));

		_rtv = CreateRenderTargetView(device, _swapchain);
	}

}

void RenderTarget::ResizeBuffer()
{
	_rtv = nullptr;

	if (_swapchain != nullptr)
	{
		RECT rect;
		DXGI_SWAP_CHAIN_DESC desc;
		HRException::CheckHR(_swapchain->GetDesc(&desc));

		::GetClientRect(_hwnd, &rect);
		HRException::CheckHR(_swapchain->ResizeBuffers(
			1, rect.right - rect.left, rect.bottom - rect.top, desc.BufferDesc.Format,
			desc.Flags));

		_rtv = CreateRenderTargetView(_device, _swapchain);
	}
}

void RenderTarget::Present()
{
	if (_swapchain != nullptr)
	{
		HRException::CheckHR(_swapchain->Present(0, 0));
	}
}
