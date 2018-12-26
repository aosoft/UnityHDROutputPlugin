#include "stdafx.h"
#include "SharedTexture.h"

SharedTexture::SharedTexture(ComPtr<ID3D11Device> const& device) :
	_device(device)
{
}

void SharedTexture::SetSourceTexture(
	ComPtr<ID3D11Device> const& sourceDevice,
	ComPtr<ID3D11Texture2D> const& sourceTexture)
{
	if (sourceTexture != _sourceTexture)
	{
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
}

void SharedTexture::UpdateTexture(ComPtr<ID3D11DeviceContext> const& dc)
{
	if (dc != nullptr &&
		_sharedTexture != nullptr &&
		_sourceTexture != nullptr)
	{
		dc->CopyResource(_sharedTexture, _sourceTexture);
		dc->Flush();
	}
}


