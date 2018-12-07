#pragma once

class RenderTarget
{
private:
	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11DeviceContext> _dc;
	ComPtr<IDXGISwapChain> _swapchain;

	ComPtr<ID3D11ShaderResourceView> _rtv;
	ComPtr<ID3D11VertexShader> _vs;
	ComPtr<ID3D11PixelShader> _ps;

};
