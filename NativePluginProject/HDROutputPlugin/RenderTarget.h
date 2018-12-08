#pragma once

#include <d3d11.h>
#include <memory>

class RenderTarget
{
private:
	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11DeviceContext> _dc;
	ComPtr<IDXGISwapChain> _swapchain;


};
