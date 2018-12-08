#include "stdafx.h"
#include "Mesh.h"

ComPtr<ID3D11InputLayout> MeshVertex::CreateInputLayout(
	ComPtr<ID3D11Device> const& device, const void *vsByteCode, size_t vsByteCodeSize)
{
	static const D3D11_INPUT_ELEMENT_DESC descs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ComPtr<ID3D11InputLayout> ret;

	HRException::CheckNull(device);
	HRException::ThrowExceptionForHR(device->CreateInputLayout(descs, _countof(descs), vsByteCode, vsByteCodeSize, &ret));

	return ret;
}

Mesh::Mesh(ComPtr<ID3D11Device> const& device,
	const MeshVertex *verticies, size_t vertCount,
	const uint16_t *indicies, size_t indexCount)
{
	ComPtr<ID3D11InputLayout> ret;

	HRException::CheckNull(device);
	HRException::ThrowExceptionForHR(
		device->CreateBuffer(
			&CD3D11_BUFFER_DESC(
				static_cast<uint32_t>(vertCount * sizeof(MeshVertex)),
				D3D11_BIND_VERTEX_BUFFER,
				D3D11_USAGE_IMMUTABLE),
			&CD3D11_SUBRESOURCE_DATA(verticies, sizeof(MeshVertex), 0), &_vertexBuffer));
	HRException::ThrowExceptionForHR(
		device->CreateBuffer(
			&CD3D11_BUFFER_DESC(
				static_cast<uint32_t>(vertCount * sizeof(uint16_t)),
				D3D11_BIND_INDEX_BUFFER,
				D3D11_USAGE_IMMUTABLE),
			&CD3D11_SUBRESOURCE_DATA(verticies, sizeof(uint16_t), 0), &_indexBuffer));
}

std::shared_ptr<Mesh> Mesh::CreateRectangleMesh(ComPtr<ID3D11Device> const& device)
{
	static const MeshVertex verticies[] =
	{
		{ ::DirectX::XMFLOAT2(-1.0f,  1.0f), ::DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ ::DirectX::XMFLOAT2( 1.0f,  1.0f), ::DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ ::DirectX::XMFLOAT2(-1.0f, -1.0f), ::DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ ::DirectX::XMFLOAT2(-1.0f,  1.0f), ::DirectX::XMFLOAT2(1.0f, 1.0f) },
	};
	static const uint16_t indicies[] = { 0, 1, 2, 1, 3, 2 };

	return std::make_shared<Mesh>(device, verticies, _countof(verticies), indicies, _countof(indicies));
}

