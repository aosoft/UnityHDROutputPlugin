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
	_device(device),
	_format(DXGI_FORMAT_UNKNOWN),
	_width(0),
	_height(0)
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

		_format = desc.BufferDesc.Format;
		_width = desc.BufferDesc.Width;
		_height = desc.BufferDesc.Height;
		_rtv = CreateRenderTargetView(device, _swapchain);
	}

}

void RenderTarget::Setup(ComPtr<ID3D11DeviceContext> const& dc, ComPtr<ID3D11Texture2D> const& source)
{
	HRException::CheckNull(dc);

	if (_width < 1 || _height < 1)
	{
		return;
	}

	auto vp = D3D11_VIEWPORT();

	if (source != nullptr)
	{
		static const float fill[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		dc->ClearRenderTargetView(_rtv, fill);

		D3D11_TEXTURE2D_DESC descSource;
		source->GetDesc(&descSource);
		if (descSource.Width < 1 || descSource.Height < 1)
		{
			return;
		}

		vp.Width = _width;
		vp.Height = static_cast<float>(_width) * descSource.Height / descSource.Width;
		if (vp.Height > _height)
		{
			vp.Width = _height * descSource.Width / descSource.Height;
			vp.Height = static_cast<float>(_height);
		}

		vp.TopLeftX = (static_cast<float>(_width) - vp.Width) / 2.0f;
		vp.TopLeftY = (static_cast<float>(_height) - vp.Height) / 2.0f;
	}
	else
	{
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.Width = _width;
		vp.Height = _height;
	}

	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	dc->RSSetViewports(1, &vp);

	dc->OMSetRenderTargets(1, &_rtv.GetInterfacePtr(), nullptr);
}

void RenderTarget::ResizeBuffer()
{
	_rtv = nullptr;

	if (_swapchain != nullptr)
	{
		RECT rect;

		::GetClientRect(_hwnd, &rect);

		uint32_t width = rect.right - rect.left;
		uint32_t height = rect.bottom - rect.top;

		HRException::CheckHR(_swapchain->ResizeBuffers(
			1, width, height, _format,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
		
		_width = width;
		_height = height;
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
