#include "stdafx.h"
#include "SharedTexture.h"

SharedTexture::SharedTexture(ComPtr<ID3D11Device> const& device) :
	_device(device)
{
}

void SharedTexture::UpdateTexture(
	ComPtr<ID3D11Device> const& sourceDevice,
	ComPtr<ID3D11Texture2D> const& sourceTexture)
{
	if (sourceDevice != _sourceDevice ||
		sourceTexture != _sourceTexture)
	{
		_sourceDevice = nullptr;
		_sourceTexture = nullptr;
		_sharedTexture = nullptr;
		_texture = nullptr;
	}

	if (sourceDevice == nullptr ||
		sourceTexture == nullptr)
	{
		return;
	}

	if (_sharedTexture != nullptr)
	{
		D3D11_TEXTURE2D_DESC desc;
		sourceTexture->GetDesc(&desc);
		desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		HRException::CheckHR(_device->CreateTexture2D(&desc, nullptr, &_texture));

		HANDLE sharedHandle;
		ComPtr<IDXGIResource> dxgiResource;
		HRException::CheckHR(_texture->QueryInterface(&dxgiResource));
		HRException::CheckHR(dxgiResource->GetSharedHandle(&sharedHandle));

		HRException::CheckHR(sourceDevice->OpenSharedResource(sharedHandle, IID_PPV_ARGS(&_sharedTexture)));
	}

	ComPtr<ID3D11DeviceContext> dc;
	sourceDevice->GetImmediateContext(&dc);
	dc->CopyResource(_sharedTexture, sourceTexture);
	dc->Flush();

	_sourceDevice = sourceDevice;
	_sourceTexture = sourceTexture;
}

