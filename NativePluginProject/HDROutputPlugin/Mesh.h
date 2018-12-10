#pragma once

#include "common.h"
#include <d3d11.h>
#include <DirectXMath.h>

#pragma pack(push)
#pragma pack(4)

struct MeshVertex
{
	::DirectX::XMFLOAT2 Pos;
	::DirectX::XMFLOAT2 UV;

	static ComPtr<ID3D11InputLayout> CreateInputLayout(
		ComPtr<ID3D11Device> const& device, const void *vsByteCode, size_t vsByteCodeSize);
};

#pragma pack(pop)

class Mesh
{
private:
	ComPtr<ID3D11Buffer> _indexBuffer;
	ComPtr<ID3D11Buffer> _vertexBuffer;

public:
	Mesh(ComPtr<ID3D11Device> const& device,
		const MeshVertex *verticies, size_t vertCount,
		const uint16_t *indicies, size_t indexCount);

	void Setup(ComPtr<ID3D11DeviceContext> const& dc);

	static std::unique_ptr<Mesh> CreateRectangleMesh(ComPtr<ID3D11Device> const& device);
};
