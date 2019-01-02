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
	_currentDisplay(nullptr),
	_format(DXGI_FORMAT_UNKNOWN),
	_width(0),
	_height(0),
	_bufferCount(0),
	_requestHDR(false),
	_availableHDR(false)
{
	ComPtr<IDXGIDevice> dxgidevice;
	ComPtr<IDXGIAdapter> adapter;

	HRException::CheckNull(device);

	HRException::CheckHR(device->QueryInterface(&dxgidevice));
	HRException::CheckHR(dxgidevice->GetParent(IID_PPV_ARGS(&adapter)));
	HRException::CheckHR(adapter->GetParent(IID_PPV_ARGS(&_factory)));
}

void RenderTarget::SetRequestHDR(bool flag)
{
	if (flag != _requestHDR)
	{
		FinalizeSwapChain();
	}
	_requestHDR = flag;
}

void RenderTarget::Setup(ComPtr<ID3D11DeviceContext> const& dc, uint32_t sourceWidth, uint32_t sourceHeight)
{
	HRException::CheckNull(dc);

	if (_width < 1 || _height < 1)
	{
		return;
	}

	if (_rtv == nullptr)
	{
		InitializeSwapChain();
	}

	auto vp = D3D11_VIEWPORT();

	if (sourceWidth > 0 && sourceHeight > 0)
	{
		static const float fill[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		dc->ClearRenderTargetView(_rtv, fill);

		vp.Width = static_cast<float>(_width);
		vp.Height = static_cast<float>(_width) * sourceHeight / sourceWidth;
		if (vp.Height > _height)
		{
			vp.Width = static_cast<float>(_height) * sourceWidth / sourceHeight;
			vp.Height = static_cast<float>(_height);
		}

		vp.TopLeftX = (static_cast<float>(_width) - vp.Width) / 2.0f;
		vp.TopLeftY = (static_cast<float>(_height) - vp.Height) / 2.0f;
	}
	else
	{
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.Width = static_cast<float>(_width);
		vp.Height = static_cast<float>(_height);
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
			_bufferCount, width, height, _format,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
		
		_width = width;
		_height = height;
		_rtv = CreateRenderTargetView(_device, _swapchain);
	}
}

bool RenderTarget::InitializeSwapChainIfDisplayChanged()
{
	if (_currentDisplay != ::MonitorFromWindow(_hwnd, MONITOR_DEFAULTTONEAREST))
	{
		InitializeSwapChain();
		return true;
	}
	return false;
}

void RenderTarget::Present()
{
	if (_swapchain != nullptr)
	{
		HRException::CheckHR(_swapchain->Present(1, 0));
	}
}

void RenderTarget::InitializeSwapChain()
{
	HRESULT hr = S_OK;
	RECT rect;
	::GetClientRect(_hwnd, &rect);

	if (_factory == nullptr)
	{
		return;
	}

	FinalizeSwapChain();

	ComPtr<IDXGIFactory2> factory2;
	hr = _factory->QueryInterface(&factory2);
	if (SUCCEEDED(hr))
	{
		auto desc = DXGI_SWAP_CHAIN_DESC1();
		desc.BufferCount = 2;
		desc.Width = rect.right - rect.left;
		desc.Height = rect.bottom - rect.top;
		desc.Format = _requestHDR ? DXGI_FORMAT_R10G10B10A2_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.SampleDesc.Count = 1;
		desc.Flags = 0;

		ComPtr<IDXGISwapChain1> swapchain1;
		if (SUCCEEDED(factory2->CreateSwapChainForHwnd(_device, _hwnd, &desc, nullptr, nullptr, &swapchain1)) &&
			swapchain1 != nullptr)
		{
			_swapchain = swapchain1;
			ComPtr<IDXGISwapChain3> swapchan3;

			if (SUCCEEDED(swapchain1->QueryInterface(&swapchan3)))
			{
				DXGI_COLOR_SPACE_TYPE colorSpace =
					_requestHDR ? DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 : DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

				UINT colorSpaceSupport = 0;
				if (SUCCEEDED(swapchan3->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport)) &&
					((colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT) == DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
				{
					if (SUCCEEDED(swapchan3->SetColorSpace1(colorSpace)))
					{
						_availableHDR = _requestHDR;
					}
					else
					{
						swapchan3->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709);
						_availableHDR = false;
					}
				}
			}
			_format = desc.Format;
			_width = desc.Width;
			_height = desc.Height;
			_bufferCount = desc.BufferCount;
		}
	}

	if (_swapchain == nullptr)
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
		desc.OutputWindow = _hwnd;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		HRException::CheckHR(_factory->CreateSwapChain(_device, &desc, &_swapchain));

		_format = desc.BufferDesc.Format;
		_width = desc.BufferDesc.Width;
		_height = desc.BufferDesc.Height;
		_bufferCount = desc.BufferCount;
	}
	_rtv = CreateRenderTargetView(_device, _swapchain);
	_currentDisplay = ::MonitorFromWindow(_hwnd, MONITOR_DEFAULTTONEAREST);
}

void RenderTarget::FinalizeSwapChain()
{
	_swapchain = nullptr;
	_currentDisplay = nullptr;
	_rtv = nullptr;
	_availableHDR = false;
}
