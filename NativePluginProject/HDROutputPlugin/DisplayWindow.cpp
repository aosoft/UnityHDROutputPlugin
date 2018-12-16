#include "stdafx.h"
#include "DisplayWindow.h"

DisplayWindow::DisplayWindow()
{
}

void DisplayWindow::InitializeInstance(
	ID3D11Device *device,
	FnDebugLog fnDebugLog,
	FnStateChangedCallback fnStateChangedCallback)
{
	HRException::CheckNull(device);

	_device = device;
	_fnDebugLog = fnDebugLog;
	_fnStateChangedCallback = fnStateChangedCallback;

	if (Create(
		nullptr, ATL::CWindow::rcDefault,
		L"Unity Preview", WS_OVERLAPPEDWINDOW) == nullptr)
	{
		HRException::CheckHR(HRESULT_FROM_WIN32(::GetLastError()));
	}

	_mesh = Mesh::CreateRectangleMesh(_device);
	_material = std::make_unique<Material>(_device);
	_renderTarget = std::make_unique<RenderTarget>(m_hWnd, _device);
}

std::shared_ptr<DisplayWindow> DisplayWindow::CreateInstance(
	ID3D11Device *device,
	FnDebugLog fnDebugLog,
	FnStateChangedCallback fnStateChangedCallback)
{
	struct DisplayWindowImpl : DisplayWindow
	{
	};

	auto ret = std::make_shared<DisplayWindowImpl>();
	ret->InitializeInstance(device, fnDebugLog, fnStateChangedCallback);
	return ret;
}

void DisplayWindow::SetRequestHDR(bool flag)
{
	bool hdr = _renderTarget->IsAvailableHDR();
	_renderTarget->SetRequestHDR(flag);
	if (hdr != _renderTarget->IsAvailableHDR())
	{
		StateChangedCallback(PluginStateChanged::CurrentHDRState);
	}
}


void DisplayWindow::Render(ComPtr<ID3D11Texture2D> const& source)
{
	if (_device == nullptr)
	{
		return;
	}

	_material->SetTexture(source);
	DrawAndPresent();
}

void DisplayWindow::OnFinalMessage(_In_ HWND /*hWnd*/)
{
	_this = nullptr;
	StateChangedCallback(PluginStateChanged::WindowClosed);
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
	if (_renderTarget != nullptr)
	{
		if (_renderTarget->InitializeSwapChainIfDisplayChanged())
		{
			DrawAndPresent();
		}
	}
	bHandled = TRUE;
	return 0;
}

LRESULT DisplayWindow::OnSize(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) try
{
	if (_renderTarget != nullptr)
	{
		_renderTarget->ResizeBuffer();
		DrawAndPresent();
	}

	bHandled = TRUE;
	return 0;
}
catch (const std::exception& e)
{
	ErrorLog(_fnDebugLog, e);
	return 0;
}

void DisplayWindow::DrawAndPresent()
{
	ComPtr<ID3D11DeviceContext> dc;
	_device->GetImmediateContext(&dc);

	dc->ClearState();

	_material->Setup(dc);
	_renderTarget->Setup(dc, _material->GetTextureDesc());
	_mesh->Draw(dc);
	_renderTarget->Present();
}
