#include "stdafx.h"
#include "Material.h"

Material::Material(ComPtr<ID3D11Device> const& device) : _device(device)
{
	HRException::CheckNull(device);
}

void Material::Setup(ComPtr<ID3D11DeviceContext> const& dc)
{
	HRException::CheckNull(dc);
}

