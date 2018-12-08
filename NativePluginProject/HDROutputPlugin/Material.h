#pragma once

#include <d3d11.h>
#include <memory>

class Material
{
private:
	ComPtr<ID3D11Device> _device;

	ComPtr<ID3D11ShaderResourceView> _rtv;

	ComPtr<ID3D11VertexShader> _vs;
	ComPtr<ID3D11PixelShader> _ps;

	ComPtr<ID3D11SamplerState> _sampler;
	ComPtr<ID3D11BlendState> _blend;
	ComPtr<ID3D11RasterizerState> _rasterizer;

public:
	Material(ComPtr<ID3D11Device> const& device);
	void Setup(ComPtr<ID3D11DeviceContext> const& dc);

};
