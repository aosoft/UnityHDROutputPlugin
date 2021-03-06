#pragma once

#include "common.h"
#include <atlwin.h>

#include <HDROutputPlugin.h>
#include "SharedTexture.h"
#include "Material.h"
#include "Mesh.h"
#include "RenderTarget.h"

class DisplayWindow :
	public ATL::CWindowImpl<DisplayWindow>,
	public std::enable_shared_from_this<DisplayWindow>
{
public:
	DECLARE_WND_CLASS(L"UnityHDROutput/DisplayWindow");

private:
	std::shared_ptr<DisplayWindow> _this;

	FnDebugLog _fnDebugLog;
	FnStateChangedCallback _fnStateChangedCallback;
	ComPtr<ID3D11Device> _device;
	PluginRect _rectWindowClosing;

	std::atomic_int32_t _updatedTextureCounter;
	int32_t _updatedTextureCounterChecker;

	std::unique_ptr<SharedTexture> _sharedTexture;
	std::unique_ptr<Mesh> _mesh;
	std::unique_ptr<Material> _material;
	std::unique_ptr<RenderTarget> _renderTarget;

	bool _convertColorSpace;

	ColorSpace _lastActiveColorSpace;
	bool _lastConvertColorSpace;

public:
	BEGIN_MSG_MAP(DisplayWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_MOVE, OnMove)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

private:
	DisplayWindow();
	void InitializeInstance(
		HWND wndParent,
		ID3D11Device *device,
		FnDebugLog fnDebugLog,
		FnStateChangedCallback fnStateChangedCallback);

public:
	static std::shared_ptr<DisplayWindow> CreateInstance(
		HWND wndParent,
		ID3D11Device *device,
		FnDebugLog fnDebugLog,
		FnStateChangedCallback fnStateChangedCallback);

	void GetWindowPluginRect(PluginRect& rect) const
	{
		if (IsWindow())
		{
			rect.UpdateFromHWnd(m_hWnd);
		}
		else
		{
			rect = _rectWindowClosing;
		}
	}

	ColorSpace GetRequestColorSpace() const noexcept
	{
		return _renderTarget->GetRequestColorSpace();
	}

	void SetRequestColorSpace(ColorSpace colorSpace);

	ColorSpace GetActiveColorSpace() const noexcept
	{
		return _renderTarget->GetActiveColorSpace();
	}

	bool GetConvertColorSpace() const
	{
		return _convertColorSpace;
	}

	void SetConvertColorSpace(bool flag)
	{
		_convertColorSpace = flag;
	}

	void SetSourceTexture(ComPtr<ID3D11Texture2D> const& source);
	void UpdateSourceTexture();

	float GetRelativeEV() const noexcept
	{
		return _material->GetRelativeEV();
	}

	void SetRelativeEV(float value) noexcept
	{
		_material->SetRelativeEV(value);
	}

	void Render();
	void RenderIfUpdatedSourceTexture();

	virtual void OnFinalMessage(_In_ HWND /*hWnd*/) override;

	LRESULT OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMove(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
	void UpdateWindowText(ColorSpace activeColorSpace, bool convertColorSpace) noexcept;

	void StateChangedCallback(PluginStateChanged state)
	{
		if (_fnStateChangedCallback != nullptr)
		{
			_fnStateChangedCallback(state);
		}
	}
};

inline void ErrorLog(FnDebugLog fnDebugLog, const std::exception& e)
{
	if (fnDebugLog != nullptr)
	{
		auto *e2 = dynamic_cast<const HRException *>(&e);
		if (e2 != nullptr)
		{
			wchar_t tmp[256];
			swprintf_s(tmp, L"An error occurred. (hr = %08x)", e2->GetResult());
			fnDebugLog(PluginLogType::Error, tmp);
		}
		else
		{
			ATL::CA2W tmp(e.what());
			fnDebugLog(PluginLogType::Error, tmp);
		}

	}
}

inline void ErrorLog(FnDebugLog fnDebugLog, const _com_error& e)
{
	if (fnDebugLog != nullptr)
	{
		wchar_t tmp[256];
		swprintf_s(tmp, L"An error occurred. (_com_error / hr = %08x)", e.Error());
		fnDebugLog(PluginLogType::Error, tmp);
	}
}
