#include "stdafx.h"
#include "DisplayWindow.h"

DisplayWindow::DisplayWindow(FnDebugLog fnDebugLog) :
	_fnDebugLog(fnDebugLog)
{
}

void DisplayWindow::InitializeD3D11(ID3D11Device *device)
{
	_device = device;

	_mesh = Mesh::CreateRectangleMesh(_device);
	_material = std::make_unique<Material>(_device);
	_renderTarget = std::make_unique<RenderTarget>(m_hWnd, _device);
}

void DisplayWindow::Render(ComPtr<ID3D11Texture2D> const& source)
{
	if (_device == nullptr)
	{
		return;
	}

	ComPtr<ID3D11DeviceContext> dc;
	_device->GetImmediateContext(&dc);

	dc->ClearState();

	if (_material != nullptr)
	{
		_material->Setup(dc, source);
	}
	if (_renderTarget != nullptr && _mesh != nullptr)
	{
		_renderTarget->Setup(dc, source);
		_mesh->Draw(dc);
		_renderTarget->Present();
	}
}

void DisplayWindow::OnFinalMessage(_In_ HWND /*hWnd*/)
{
	_this = nullptr;
}


LRESULT DisplayWindow::OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	_this = shared_from_this();
	bHandled = TRUE;
	return 0;
}

LRESULT DisplayWindow::OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	return 0;
}

LRESULT DisplayWindow::OnMove(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	return 0;
}

LRESULT DisplayWindow::OnSize(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) try
{
	if (_renderTarget != nullptr)
	{
		_renderTarget->ResizeBuffer();
	}

	bHandled = TRUE;
	return 0;
}
catch (const std::exception& e)
{
	ErrorLog(_fnDebugLog, e);
	return 0;
}
