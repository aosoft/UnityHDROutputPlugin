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

public:
	RenderTarget(HWND hwnd, ComPtr<ID3D11Device> const& device);

};
