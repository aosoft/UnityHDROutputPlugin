#include "stdafx.h"
#include "SharedTexture.h"

SharedTexture::SharedTexture(ComPtr<ID3D11Device> const& device) :
	_device(device)
{
}

void SharedTexture::SetSourceTexture(ComPtr<ID3D11Texture2D> const& sourceTexture) try
{
	if (sourceTexture == _sourceTexture)
	{
		return;
	}

	_sourceTexture = nullptr;

	if (sourceTexture == nullptr)
	{
		_sourceDevice = nullptr;
		_sourceDC = nullptr;
		_sharedTexture = nullptr;
		_texture = nullptr;
		return;
	}

	_sourceDevice = nullptr;
	_sourceDC = nullptr;
	_sharedTexture = nullptr;
	_texture = nullptr;

	D3D11_TEXTURE2D_DESC desc;
	sourceTexture->GetDesc(&desc);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;

	if (_texture != nullptr)
	{
		D3D11_TEXTURE2D_DESC descTemp;
		_texture->GetDesc(&descTemp);
		if (!memcmp(&desc, &descTemp, sizeof(desc)))
		{
			return;
		}
	}

	ComPtr<ID3D11Device> sourceDevice;
	sourceTexture->GetDevice(&sourceDevice);

	HRException::CheckHR(_device->CreateTexture2D(&desc, nullptr, &_texture));

	HANDLE sharedHandle;
	ComPtr<IDXGIResource> dxgiResource;
	HRException::CheckHR(_texture->QueryInterface(&dxgiResource));
	HRException::CheckHR(dxgiResource->GetSharedHandle(&sharedHandle));

	HRException::CheckHR(sourceDevice->OpenSharedResource(sharedHandle, IID_PPV_ARGS(&_sharedTexture)));

	_sourceDevice = sourceDevice;
	sourceDevice->GetImmediateContext(&_sourceDC);
	_sourceTexture = sourceTexture;
}
catch (...)
{
	_sourceDevice = nullptr;
	_sourceDC = nullptr;
	_sharedTexture = nullptr;
	_texture = nullptr;
	throw;
}

void SharedTexture::UpdateTexture()
{
	if (_sourceDC != nullptr &&
		_sharedTexture != nullptr &&
		_sourceTexture != nullptr)
	{
		_sourceDC->CopyResource(_sharedTexture, _sourceTexture);
		_sourceDC->Flush();
	}
}


