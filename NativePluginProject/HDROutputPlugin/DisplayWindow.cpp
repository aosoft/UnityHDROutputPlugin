#include "stdafx.h"
#include "DisplayWindow.h"

DisplayWindow::DisplayWindow() :
	_rectWindowClosing(),
	_updatedTextureCounter(0),
	_updatedTextureCounterChecker(0),
	_gammaCollect(false),
	_topmost(false)
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

	_sharedTexture = std::make_unique<SharedTexture>(_device);
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
		StateChangedCallback(PluginStateChanged::CurrentHDRStateChanged);
	}
}

void DisplayWindow::SetTopmost(bool flag)
{
	if (flag == _topmost)
	{
		return;
	}

	SetWindowPos(flag ? HWND_TOPMOST : HWND_NOTOPMOST,
		0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

	_topmost = flag;
}

void DisplayWindow::SetSourceTexture(ComPtr<ID3D11Texture2D> const& source)
{
	_sharedTexture->SetSourceTexture(source);
	_material->SetTexture(_sharedTexture->GetTexture());
	UpdateSourceTexture();
}

void DisplayWindow::UpdateSourceTexture()
{
	_sharedTexture->UpdateTexture();
	++_updatedTextureCounter;
}

void DisplayWindow::Render()
{
	ComPtr<ID3D11DeviceContext> dc;
	_device->GetImmediateContext(&dc);

	dc->ClearState();

	_material->Setup(
		dc,
		_gammaCollect ? _renderTarget->IsAvailableHDR() ?
		PSCode::LinearToBT2100PQ : PSCode::LinearToSRGB : PSCode::PassThrough);
	_renderTarget->Setup(dc, _material->GetTextureDesc());
	_mesh->Draw(dc);
	_renderTarget->Present();
}

void DisplayWindow::RenderIfUpdatedSourceTexture()
{
	int32_t c = _updatedTextureCounter;
	if (c != _updatedTextureCounterChecker)
	{
		Render();
	}
	_updatedTextureCounter = c;
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

LRESULT DisplayWindow::OnClose(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	_rectWindowClosing.UpdateFromHWnd(m_hWnd);

	_mesh = nullptr;
	_material = nullptr;
	_renderTarget = nullptr;

	bHandled = FALSE;
	return 0;
}

LRESULT DisplayWindow::OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::PostQuitMessage(0);
	bHandled = TRUE;
	return 0;
}

LRESULT DisplayWindow::OnMove(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (_renderTarget != nullptr)
	{
		bool hdr = _renderTarget->IsAvailableHDR();
		if (_renderTarget->InitializeSwapChainIfDisplayChanged())
		{
			if (hdr != _renderTarget->IsAvailableHDR())
			{
				StateChangedCallback(PluginStateChanged::CurrentHDRStateChanged);
				Render();
			}
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
		Render();
	}

	bHandled = TRUE;
	return 0;
}
catch (const std::exception& e)
{
	ErrorLog(_fnDebugLog, e);
	return 0;
}
catch (const _com_error& e)
{
	ErrorLog(_fnDebugLog, e);
	return 0;
}
