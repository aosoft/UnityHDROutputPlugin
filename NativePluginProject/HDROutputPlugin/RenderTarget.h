#pragma once

#include "common.h"
#include <d3d11.h>
#include <dxgi1_6.h>

enum class InitializeSwapChainResult : int32_t
{
	Initialized = 0,
	Uninitialized,
	DoNotDisplayChanged
};

enum class ColorSpace : int32_t
{
	sRGB = 0,
	BT2100_PQ,
	BT709_Linear
};

class RenderTarget
{
private:
	ComPtr<ID3D11Device> _device;

	HWND _hwnd;
	ComPtr<IDXGIFactory> _factory;
	ComPtr<IDXGIAdapter> _adapter;
	ComPtr<IDXGISwapChain> _swapchain;
	ComPtr<ID3D11RenderTargetView> _rtv;
	HMONITOR _currentDisplay;

	DXGI_FORMAT _format;
	uint32_t _width;
	uint32_t _height;
	uint32_t _bufferCount;

	ColorSpace _requestColorSpace;
	ColorSpace _activeColorSpace;

public:
	RenderTarget(HWND hwnd, ComPtr<ID3D11Device> const& device);

	ColorSpace GetRequestColorSpace() const noexcept
	{
		return _requestColorSpace;
	}

	void SetRequestColorSpace(ColorSpace colorSpace);

	ColorSpace GetActiveColorSpace() const noexcept
	{
		return _activeColorSpace;
	}

	InitializeSwapChainResult CheckAndInitializeSwapChain();
	InitializeSwapChainResult InitializeSwapChainIfDisplayChanged();

	void Setup(ComPtr<ID3D11DeviceContext> const& dc, uint32_t sourceWidth, uint32_t sourceHeight);

	void Setup(ComPtr<ID3D11DeviceContext> const& dc, const D3D11_TEXTURE2D_DESC& sourceDesc)
	{
		Setup(dc, sourceDesc.Width, sourceDesc.Height);
	}


	void ResizeBuffer();
	void Present();

private:
	InitializeSwapChainResult InitializeSwapChain();
	void FinalizeSwapChain();

};
