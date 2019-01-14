#include "stdafx.h"
#include "Material.h"
#include "vs.csh"
#include "ps_Through.csh"
#include "ps_LinearToSRGB.csh"
#include "ps_LinearToBT2100PQ.csh"

Material::Material(ComPtr<ID3D11Device> const& device) :
	_device(device),
	_descTexture(),
	_constants(),
	_pscode(PSCode::PassThrough)
{
	HRException::CheckNull(device);

	_layout = MeshVertex::CreateInputLayout(device, g_shaderbin_vs, sizeof(g_shaderbin_vs));
	HRException::CheckHR(device->CreateVertexShader(g_shaderbin_vs, sizeof(g_shaderbin_vs), nullptr, &_vs));

	HRException::CheckHR(device->CreateBuffer(
		&CD3D11_BUFFER_DESC(sizeof(_constants), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE),
		nullptr, &_constantsBuffer));
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

		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);
		CD3D11_SHADER_RESOURCE_VIEW_DESC vdesc(D3D11_SRV_DIMENSION_TEXTURE2D, desc.Format);

		switch (desc.Format)
		{
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
			vdesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;

		case DXGI_FORMAT_R32G32B32_TYPELESS:
			vdesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			break;

		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
			vdesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			break;

		case DXGI_FORMAT_R32G32_TYPELESS:
			vdesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			break;

		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
			vdesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
			break;

		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			vdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;

		case DXGI_FORMAT_R16G16_TYPELESS:
			vdesc.Format = DXGI_FORMAT_R16G16_FLOAT;
			break;

		case DXGI_FORMAT_R32_TYPELESS:
			vdesc.Format = DXGI_FORMAT_R32_FLOAT;
			break;

		case DXGI_FORMAT_R8G8_TYPELESS:
			vdesc.Format = DXGI_FORMAT_R8G8_UNORM;
			break;

		case DXGI_FORMAT_R16_TYPELESS:
			vdesc.Format = DXGI_FORMAT_R16_FLOAT;
			break;

		case DXGI_FORMAT_R8_TYPELESS:
			vdesc.Format = DXGI_FORMAT_R8_UNORM;
			break;

		case DXGI_FORMAT_BC1_TYPELESS:
			vdesc.Format = DXGI_FORMAT_BC1_UNORM;
			break;

		case DXGI_FORMAT_BC2_TYPELESS:
			vdesc.Format = DXGI_FORMAT_BC2_UNORM;
			break;

		case DXGI_FORMAT_BC3_TYPELESS:
			vdesc.Format = DXGI_FORMAT_BC3_UNORM;
			break;

		case DXGI_FORMAT_BC4_TYPELESS:
			vdesc.Format = DXGI_FORMAT_BC4_UNORM;
			break;

		case DXGI_FORMAT_BC5_TYPELESS:
			vdesc.Format = DXGI_FORMAT_BC5_UNORM;
			break;

		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			vdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			break;

		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
			vdesc.Format = DXGI_FORMAT_B8G8R8X8_UNORM;
			break;

		case DXGI_FORMAT_BC6H_TYPELESS:
			vdesc.Format = DXGI_FORMAT_BC6H_UF16;
			break;

		case DXGI_FORMAT_BC7_TYPELESS:
			vdesc.Format = DXGI_FORMAT_BC7_UNORM;
			break;
		};

		_device->CreateShaderResourceView(texture, &vdesc, &_srv);
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

void Material::Setup(ComPtr<ID3D11DeviceContext> const& dc, PSCode pscode)
{
	HRException::CheckNull(dc);

	dc->IASetInputLayout(_layout);

	dc->VSSetShader(_vs, nullptr, 0);

	if (_ps == nullptr || pscode != _pscode)
	{
		_ps = nullptr;

		switch (pscode)
		{
		case PSCode::LinearToSRGB:
			HRException::CheckHR(_device->CreatePixelShader(g_shaderbin_ps_LinearToSRGB, sizeof(g_shaderbin_ps_LinearToSRGB), nullptr, &_ps));
			break;

		case PSCode::LinearToBT2100PQ:
			HRException::CheckHR(_device->CreatePixelShader(g_shaderbin_ps_LinearToBT2100PQ, sizeof(g_shaderbin_ps_LinearToBT2100PQ), nullptr, &_ps));
			break;

		default:
			HRException::CheckHR(_device->CreatePixelShader(g_shaderbin_ps_Through, sizeof(g_shaderbin_ps_Through), nullptr, &_ps));
		};

		_pscode = pscode;
	}

	D3D11_MAPPED_SUBRESOURCE resource;
	HRException::CheckHR(dc->Map(_constantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource));
	memcpy(resource.pData, &_constants, sizeof(_constants));
	dc->Unmap(_constantsBuffer, 0);

	dc->PSSetShader(_ps, nullptr, 0);
	dc->PSSetSamplers(0, 1, &_sampler.GetInterfacePtr());
	dc->PSSetConstantBuffers(0, 1, &_constantsBuffer.GetInterfacePtr());

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

