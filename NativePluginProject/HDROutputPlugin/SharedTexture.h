#pragma once

#include "common.h"
#include <d3d11.h>

class SharedTexture
{
private:
	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11Texture2D> _texture;

	ComPtr<ID3D11Device> _sourceDevice;
	ComPtr<ID3D11DeviceContext> _sourceDC;
	ComPtr<ID3D11Texture2D> _sharedTexture;
	ComPtr<ID3D11Texture2D> _sourceTexture;


public:
	SharedTexture(ComPtr<ID3D11Device> const& device);

	ComPtr<ID3D11Texture2D> const& GetTexture() const noexcept
	{
		return _texture;
	}

	void SetSourceTexture(ComPtr<ID3D11Texture2D> const& sourceTexture);

	void UpdateTexture();
};
