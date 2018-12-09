#pragma once

#include "common.h"
#include <d3d11.h>

#include "Mesh.h"

class Material
{
private:
	ComPtr<ID3D11Device> _device;

	ComPtr<ID3D11InputLayout> _layout;
	ComPtr<ID3D11VertexShader> _vs;
	ComPtr<ID3D11PixelShader> _ps;

	ComPtr<ID3D11SamplerState> _sampler;
	ComPtr<ID3D11BlendState> _blend;
	ComPtr<ID3D11RasterizerState> _rasterizer;

public:
	Material(ComPtr<ID3D11Device> const& device);
	void Setup(ComPtr<ID3D11DeviceContext> const& dc, ComPtr<ID3D11Texture2D> const& source);

};
