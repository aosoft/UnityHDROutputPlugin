#include "stdafx.h"
#include "Material.h"
#include "vs.csh"
#include "ps_Through.csh"

Material::Material(ComPtr<ID3D11Device> const& device) : _device(device)
{
	HRException::CheckNull(device);

	_layout = MeshVertex::CreateInputLayout(device, g_shaderbin_vs, sizeof(g_shaderbin_vs));
	HRException::CheckHR(device->CreateVertexShader(g_shaderbin_vs, sizeof(g_shaderbin_vs), nullptr, &_vs));

	HRException::CheckHR(device->CreateSamplerState(&CD3D11_SAMPLER_DESC(CD3D11_DEFAULT()), &_sampler));
	HRException::CheckHR(device->CreateBlendState(&CD3D11_BLEND_DESC(CD3D11_DEFAULT()), &_blend));
	
	auto culldesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
	culldesc.CullMode = D3D11_CULL_NONE;
	HRException::CheckHR(device->CreateRasterizerState(&culldesc, &_rasterizer));
}

void Material::Setup(ComPtr<ID3D11DeviceContext> const& dc)
{
	HRException::CheckNull(dc);
}

