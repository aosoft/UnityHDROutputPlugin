#pragma once

#include "common.h"
#include <d3d11.h>
#include <dxgi1_4.h>

class RenderTarget
{
private:
	ComPtr<ID3D11Device> _device;

	HWND _hwnd;
	ComPtr<IDXGIFactory> _factory;
	ComPtr<IDXGIFactory2> _factory2;
	ComPtr<IDXGISwapChain> _swapchain;
	ComPtr<IDXGISwapChain1> _swapchain1;
	ComPtr<ID3D11RenderTargetView> _rtv;

	DXGI_FORMAT _format;
	uint32_t _width;
	uint32_t _height;

public:
	RenderTarget(HWND hwnd, ComPtr<ID3D11Device> const& device);

	ComPtr<ID3D11RenderTargetView> const& GetRenderTargetView()
	{
		return _rtv;
	}

	void Setup(ComPtr<ID3D11DeviceContext> const& dc, uint32_t sourceWidth, uint32_t sourceHeight);

	void Setup(ComPtr<ID3D11DeviceContext> const& dc, const D3D11_TEXTURE2D_DESC& sourceDesc)
	{
		Setup(dc, sourceDesc.Width, sourceDesc.Height);
	}


	void ResizeBuffer();
	void Present();
};
