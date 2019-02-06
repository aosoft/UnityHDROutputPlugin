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
	_requestColorSpace(ColorSpace::sRGB),
	_activeColorSpace(ColorSpace::sRGB)
{
	ComPtr<IDXGIDevice> dxgidevice;

	HRException::CheckNull(device);

	HRException::CheckHR(device->QueryInterface(&dxgidevice));
	HRException::CheckHR(dxgidevice->GetParent(IID_PPV_ARGS(&_adapter)));
	HRException::CheckHR(_adapter->GetParent(IID_PPV_ARGS(&_factory)));
}

void RenderTarget::SetRequestColorSpace(ColorSpace colorSpace)
{
	if (colorSpace != _requestColorSpace)
	{
		FinalizeSwapChain();
	}
	_requestColorSpace = colorSpace;
}

InitializeSwapChainResult RenderTarget::CheckAndInitializeSwapChain()
{
	if (_width < 1 || _height < 1 || _rtv == nullptr)
	{
		return InitializeSwapChain();
	}
	return InitializeSwapChainResult::Initialized;
}

InitializeSwapChainResult RenderTarget::InitializeSwapChainIfDisplayChanged()
{
	if (_currentDisplay != ::MonitorFromWindow(_hwnd, MONITOR_DEFAULTTONEAREST))
	{
		return InitializeSwapChain();
	}
	return InitializeSwapChainResult::DoNotDisplayChanged;
}

void RenderTarget::Setup(ComPtr<ID3D11DeviceContext> const& dc, uint32_t sourceWidth, uint32_t sourceHeight)
{
	HRException::CheckNull(dc);

	if (_width < 1 || _height < 1 || _rtv == nullptr)
	{
		return;
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

		if (width > 0 && height > 0)
		{
			HRException::CheckHR(_swapchain->ResizeBuffers(
				_bufferCount, width, height, _format,
				DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
			_rtv = CreateRenderTargetView(_device, _swapchain);
		}
		
		_width = width;
		_height = height;
	}
}

void RenderTarget::Present()
{
	if (_swapchain != nullptr)
	{
		HRException::CheckHR(_swapchain->Present(1, 0));
	}
}

InitializeSwapChainResult RenderTarget::InitializeSwapChain()
{
	HRESULT hr = S_OK;

	RECT rect;
	::GetClientRect(_hwnd, &rect);
	uint32_t width = rect.right - rect.left;
	uint32_t height = rect.bottom - rect.top;

	FinalizeSwapChain();

	HMONITOR currentDisplay = ::MonitorFromWindow(_hwnd, MONITOR_DEFAULTTONEAREST);

	if (width < 1 || height < 1)
	{
		_currentDisplay = currentDisplay;
		return InitializeSwapChainResult::Uninitialized;
	}

	ComPtr<IDXGIFactory2> factory2;
	hr = _factory->QueryInterface(&factory2);
	if (SUCCEEDED(hr))
	{
		auto colorSpace = ColorSpace::sRGB;
		if (_requestColorSpace != ColorSpace::sRGB)
		{
			for (UINT i = 0; true; i++)
			{
				ComPtr<IDXGIOutput> output;
				if (FAILED(_adapter->EnumOutputs(i, &output)))
				{
					break;
				}

				ComPtr<IDXGIOutput6> output6;
				if (FAILED(output->QueryInterface(&output6)))
				{
					break;
				}

				DXGI_OUTPUT_DESC1 desc;
				if (SUCCEEDED(output6->GetDesc1(&desc)) &&
					desc.Monitor == currentDisplay &&
					desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
				{
					colorSpace = _requestColorSpace;
					break;
				}
			}
		}

		auto desc = DXGI_SWAP_CHAIN_DESC1();
		DXGI_COLOR_SPACE_TYPE colorSpaceDXGI = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

		desc.BufferCount = 2;
		desc.Width = width;
		desc.Height = height;

		switch (colorSpace)
		{
		case ColorSpace::BT2100_PQ:
			desc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
			colorSpaceDXGI = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
			break;

		case ColorSpace::BT709_Linear:
			desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			colorSpaceDXGI = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
			break;

		default:
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			colorSpaceDXGI = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
			break;
		};

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
				UINT colorSpaceSupport = 0;
				if (SUCCEEDED(swapchan3->CheckColorSpaceSupport(colorSpaceDXGI, &colorSpaceSupport)) &&
					((colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT) == DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
				{
					if (SUCCEEDED(swapchan3->SetColorSpace1(colorSpaceDXGI)))
					{
						_activeColorSpace = colorSpace;
					}
					else
					{
						swapchan3->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709);
						_activeColorSpace = ColorSpace::sRGB;
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
		desc.BufferDesc.Width = width;
		desc.BufferDesc.Height = height;
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
	_currentDisplay = currentDisplay;

	return InitializeSwapChainResult::Initialized;
}

void RenderTarget::FinalizeSwapChain()
{
	_swapchain = nullptr;
	_currentDisplay = nullptr;
	_rtv = nullptr;
	_activeColorSpace = ColorSpace::sRGB;
	_format = DXGI_FORMAT_UNKNOWN;
	_width = 0;
	_height = 0;
}
