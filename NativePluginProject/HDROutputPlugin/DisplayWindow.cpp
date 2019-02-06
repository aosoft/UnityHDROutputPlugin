#include "stdafx.h"
#include "DisplayWindow.h"

DisplayWindow::DisplayWindow() :
	_rectWindowClosing(),
	_updatedTextureCounter(0),
	_updatedTextureCounterChecker(0),
	_convertColorSpace(false),
	_lastActiveColorSpace(ColorSpace::sRGB),
	_lastConvertColorSpace(false)
{
}

void DisplayWindow::InitializeInstance(
	HWND wndParent,
	ID3D11Device *device,
	FnDebugLog fnDebugLog,
	FnStateChangedCallback fnStateChangedCallback)
{
	HRException::CheckNull(device);

	_device = device;
	_fnDebugLog = fnDebugLog;
	_fnStateChangedCallback = fnStateChangedCallback;

	if (Create(
		wndParent, ATL::CWindow::rcDefault,
		L"", WS_OVERLAPPEDWINDOW) == nullptr)
	{
		HRException::CheckHR(HRESULT_FROM_WIN32(::GetLastError()));
	}

	_sharedTexture = std::make_unique<SharedTexture>(_device);
	_mesh = Mesh::CreateRectangleMesh(_device);
	_material = std::make_unique<Material>(_device);
	_renderTarget = std::make_unique<RenderTarget>(m_hWnd, _device);

	_lastActiveColorSpace = _renderTarget->GetActiveColorSpace();
	_lastConvertColorSpace = _convertColorSpace;
	UpdateWindowText(_lastActiveColorSpace, _lastConvertColorSpace);
}

std::shared_ptr<DisplayWindow> DisplayWindow::CreateInstance(
	HWND wndParent,
	ID3D11Device *device,
	FnDebugLog fnDebugLog,
	FnStateChangedCallback fnStateChangedCallback)
{
	struct DisplayWindowImpl : DisplayWindow
	{
	};

	auto ret = std::make_shared<DisplayWindowImpl>();
	ret->InitializeInstance(wndParent, device, fnDebugLog, fnStateChangedCallback);
	return ret;
}

void DisplayWindow::SetRequestColorSpace(ColorSpace colorSpace)
{
	auto current = _renderTarget->GetActiveColorSpace();
	_renderTarget->SetRequestColorSpace(colorSpace);
	if (current != _renderTarget->GetActiveColorSpace())
	{
		StateChangedCallback(PluginStateChanged::CurrentHDRStateChanged);
	}
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
	if (_renderTarget->CheckAndInitializeSwapChain() == InitializeSwapChainResult::Uninitialized)
	{
		return;
	}

	ComPtr<ID3D11DeviceContext> dc;
	_device->GetImmediateContext(&dc);
	auto activeColorSpace = _renderTarget->GetActiveColorSpace();
	bool convertColorSpace = _convertColorSpace;

	dc->ClearState();

	static const PSCode pscodes[] =
	{
		PSCode::LinearToSRGB,
		PSCode::LinearToBT2100PQ,
		PSCode::PassThrough
	};

	_material->Setup(
		dc,
		convertColorSpace ?
			pscodes[static_cast<size_t>(activeColorSpace)] :
			PSCode::PassThrough);
	_renderTarget->Setup(dc, _material->GetTextureDesc());
	_mesh->Draw(dc);
	_renderTarget->Present();

	if (convertColorSpace != _lastConvertColorSpace ||
		activeColorSpace != _lastActiveColorSpace)
	{
		if (activeColorSpace != _lastActiveColorSpace)
		{
			StateChangedCallback(PluginStateChanged::CurrentHDRStateChanged);
		}
		UpdateWindowText(activeColorSpace, convertColorSpace);
		_lastConvertColorSpace = convertColorSpace;
		_lastActiveColorSpace = activeColorSpace;
	}
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
		auto activeColorSpace = _renderTarget->GetActiveColorSpace();
		if (_renderTarget->InitializeSwapChainIfDisplayChanged() == InitializeSwapChainResult::Initialized)
		{
			if (activeColorSpace != _renderTarget->GetActiveColorSpace())
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

void DisplayWindow::UpdateWindowText(ColorSpace activeColorSpace, bool convertColorSpace) noexcept try
{
	const wchar_t *colorSpaceName = L"";
	const wchar_t *convert = L"None (Pass through)";
	switch (activeColorSpace)
	{
	case ColorSpace::BT2100_PQ:
		colorSpaceName = L"HDR / BT.2100 (PQ)";
		if (convertColorSpace)
		{
			convert = L"Linear -> BT.2100 (PQ)";
		}
		break;

	case ColorSpace::BT709_Linear:
		colorSpaceName = L"HDR / BT.709 (Linear)";
		if (convertColorSpace)
		{
			convert = L"None (BT.709 (Linear))";
		}
		break;

	default:
		colorSpaceName = L"SDR / sRGB";
		if (convertColorSpace)
		{
			convert = L"Linear -> sRGB";
		}
	};

	wchar_t tmp[256];
	swprintf_s(tmp, L"Unity Preview [Output:%s, Convert: %s]", colorSpaceName, convert);

	SetWindowText(tmp);
}
catch (const std::exception&)
{
	SetWindowText(L"");
	return;
}
