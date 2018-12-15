#pragma once

#include "common.h"
#include <atlwin.h>

#include <HDROutputPlugin.h>
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
	ComPtr<ID3D11Device> _device;

	std::unique_ptr<Mesh> _mesh;
	std::unique_ptr<Material> _material;
	std::unique_ptr<RenderTarget> _renderTarget;

public:
	BEGIN_MSG_MAP(DisplayWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_MOVE, OnMove)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

private:
	DisplayWindow();
	void InitializeInstance(ID3D11Device *device, FnDebugLog fnDebugLog);

public:
	static std::shared_ptr<DisplayWindow> CreateInstance(ID3D11Device *device, FnDebugLog fnDebugLog);

	bool GetRequestHDR() const
	{
		return _renderTarget->GetRequestHDR();
	}

	void SetRequestHDR(bool flag)
	{
		_renderTarget->SetRequestHDR(flag);
	}

	bool IsAvailableHDR() const
	{
		return _renderTarget->IsAvailableHDR();
	}

	void Render(ComPtr<ID3D11Texture2D> const& source);

	virtual void OnFinalMessage(_In_ HWND /*hWnd*/) override;

	LRESULT OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMove(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
	void DrawAndPresent();
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
