#pragma once

#include "common.h"
#include <d3d11.h>

class SharedTexture
{
private:
	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11Texture2D> _texture;

	ComPtr<ID3D11Device> _sourceDevice;
	ComPtr<ID3D11Texture2D> _sourceTexture;
	ComPtr<ID3D11Texture2D> _sharedTexture;


public:
	SharedTexture(ComPtr<ID3D11Device> const& device);

	ComPtr<ID3D11Texture2D> const& GetTexture() const noexcept
	{
		return _texture;
	}

	void UpdateTexture(
		ComPtr<ID3D11Device> const& sourceDevice,
		ComPtr<ID3D11Texture2D> const& sourceTexture);
};
