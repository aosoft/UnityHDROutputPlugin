#include "stdafx.h"
#include "Material.h"
#include "vs.csh"
#include "ps_Through.csh"
#include "ps_LinearToSRGB.csh"
#include "ps_LinearToBT2100PQ.csh"

Material::Material(ComPtr<ID3D11Device> const& device) :
	_device(device),
	_descTexture()
{
	HRException::CheckNull(device);

	_layout = MeshVertex::CreateInputLayout(device, g_shaderbin_vs, sizeof(g_shaderbin_vs));
	HRException::CheckHR(device->CreateVertexShader(g_shaderbin_vs, sizeof(g_shaderbin_vs), nullptr, &_vs));
	//HRException::CheckHR(device->CreatePixelShader(g_shaderbin_ps_Through, sizeof(g_shaderbin_ps_Through), nullptr, &_ps));
	HRException::CheckHR(device->CreatePixelShader(g_shaderbin_ps_LinearToSRGB, sizeof(g_shaderbin_ps_LinearToSRGB), nullptr, &_ps));

	HRException::CheckHR(device->CreateSamplerState(&CD3D11_SAMPLER_DESC(CD3D11_DEFAULT()), &_sampler));
	HRException::CheckHR(device->CreateBlendState(&CD3D11_BLEND_DESC(CD3D11_DEFAULT()), &_blend));
	
	auto culldesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
	culldesc.CullMode = D3D11_CULL_NONE;
	HRException::CheckHR(device->CreateRasterizerState(&culldesc, &_rasterizer));
}

void Material::SetTexture(ComPtr<ID3D11Texture2D> const& texture)
{
	if (texture != _texture)
	{
		_srv = nullptr;
		_texture = nullptr;

		_device->CreateShaderResourceView(
			texture,
			&CD3D11_SHADER_RESOURCE_VIEW_DESC(D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM),
			&_srv);
		_texture = texture;

		if (texture != nullptr)
		{
			texture->GetDesc(&_descTexture);
		}
		else
		{
			_descTexture = D3D11_TEXTURE2D_DESC();
		}
	}
}

void Material::Setup(ComPtr<ID3D11DeviceContext> const& dc)
{
	HRException::CheckNull(dc);

	dc->IASetInputLayout(_layout);

	dc->VSSetShader(_vs, nullptr, 0);

	dc->PSSetShader(_ps, nullptr, 0);
	dc->PSSetSamplers(0, 1, &_sampler.GetInterfacePtr());

	if (_srv != nullptr)
	{
		dc->PSSetShaderResources(0, 1, &_srv.GetInterfacePtr());
	}
	else
	{
		dc->PSSetShaderResources(0, 0, nullptr);
	}
	dc->RSSetState(_rasterizer);

	static const FLOAT blendFactor[4] = { 0 };
	dc->OMSetBlendState(_blend, blendFactor, 0xffffffff);
}

