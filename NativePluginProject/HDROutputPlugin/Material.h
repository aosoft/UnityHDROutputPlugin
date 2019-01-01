#pragma once

#include "common.h"
#include <d3d11.h>

#include "Mesh.h"

enum class PSCode : int32_t
{
	PassThrough = 0,
	LinearToSRGB,
	LinearToBT2100PQ
};

class Material
{
private:
	ComPtr<ID3D11Device> _device;

	ComPtr<ID3D11InputLayout> _layout;
	ComPtr<ID3D11VertexShader> _vs;
	ComPtr<ID3D11PixelShader> _ps;
	PSCode _pscode;

	D3D11_TEXTURE2D_DESC _descTexture;
	ComPtr<ID3D11Texture2D> _texture;
	ComPtr<ID3D11ShaderResourceView> _srv;

	ComPtr<ID3D11SamplerState> _sampler;
	ComPtr<ID3D11BlendState> _blend;
	ComPtr<ID3D11RasterizerState> _rasterizer;

public:
	Material(ComPtr<ID3D11Device> const& device);

	ComPtr<ID3D11Texture2D> const& GetTexture()
	{
		return _texture;
	}
	const D3D11_TEXTURE2D_DESC& GetTextureDesc() const noexcept
	{
		return _descTexture;
	}

	void SetTexture(ComPtr<ID3D11Texture2D> const& texture);

	void Setup(ComPtr<ID3D11DeviceContext> const& dc, PSCode pscode);
};
