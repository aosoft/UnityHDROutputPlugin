#pragma once

class RenderTarget
{
private:
	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11DeviceContext> _dc;
	ComPtr<IDXGISwapChain> _swapchain;


};
