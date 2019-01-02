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
	ComPtr<IDXGISwapChain> _swapchain;
	ComPtr<ID3D11RenderTargetView> _rtv;
	HMONITOR _currentDisplay;

	DXGI_FORMAT _format;
	uint32_t _width;
	uint32_t _height;
	uint32_t _bufferCount;

	bool _requestHDR;
	bool _availableHDR;

public:
	RenderTarget(HWND hwnd, ComPtr<ID3D11Device> const& device);

	bool GetRequestHDR() const noexcept
	{
		return _requestHDR;
	}

	void SetRequestHDR(bool flag);

	bool IsAvailableHDR() const noexcept
	{
		return _availableHDR;
	}

	void Setup(ComPtr<ID3D11DeviceContext> const& dc, uint32_t sourceWidth, uint32_t sourceHeight);

	void Setup(ComPtr<ID3D11DeviceContext> const& dc, const D3D11_TEXTURE2D_DESC& sourceDesc)
	{
		Setup(dc, sourceDesc.Width, sourceDesc.Height);
	}


	void ResizeBuffer();
	bool InitializeSwapChainIfDisplayChanged();
	void Present();

private:
	void InitializeSwapChain();
	void FinalizeSwapChain();

};
