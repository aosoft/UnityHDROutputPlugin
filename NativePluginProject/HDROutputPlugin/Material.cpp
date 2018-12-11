#include "stdafx.h"
#include "Material.h"
#include "vs.csh"
#include "ps_Through.csh"

Material::Material(ComPtr<ID3D11Device> const& device) : _device(device)
{
	HRException::CheckNull(device);

	_layout = MeshVertex::CreateInputLayout(device, g_shaderbin_vs, sizeof(g_shaderbin_vs));
	HRException::CheckHR(device->CreateVertexShader(g_shaderbin_vs, sizeof(g_shaderbin_vs), nullptr, &_vs));
	HRException::CheckHR(device->CreatePixelShader(g_shaderbin_ps_Through, sizeof(g_shaderbin_ps_Through), nullptr, &_ps));

	HRException::CheckHR(device->CreateSamplerState(&CD3D11_SAMPLER_DESC(CD3D11_DEFAULT()), &_sampler));
	HRException::CheckHR(device->CreateBlendState(&CD3D11_BLEND_DESC(CD3D11_DEFAULT()), &_blend));
	
	auto culldesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
	culldesc.CullMode = D3D11_CULL_NONE;
	HRException::CheckHR(device->CreateRasterizerState(&culldesc, &_rasterizer));
}

void Material::Setup(ComPtr<ID3D11DeviceContext> const& dc, ComPtr<ID3D11Texture2D> const& texture)
{
	HRException::CheckNull(dc);

	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11ShaderResourceView> srv;
	dc->GetDevice(&device);

	dc->IASetInputLayout(_layout);

	dc->VSSetShader(_vs, nullptr, 0);

	dc->PSSetShader(_ps, nullptr, 0);
	dc->PSSetSamplers(0, 1, &_sampler.GetInterfacePtr());

	if (texture != nullptr)
	{
		device->CreateShaderResourceView(
			texture,
			&CD3D11_SHADER_RESOURCE_VIEW_DESC(D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM),
			&srv);
		dc->PSSetShaderResources(0, 1, &srv.GetInterfacePtr());
	}

	dc->RSSetState(_rasterizer);

	static const FLOAT blendFactor[4] = { 0 };
	dc->OMSetBlendState(_blend, blendFactor, 0xffffffff);
}

