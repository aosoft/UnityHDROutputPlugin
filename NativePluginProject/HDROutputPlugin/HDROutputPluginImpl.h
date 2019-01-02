#pragma once

#include "common.h"
#include <HDROutputPlugin.h>

#include "App.h"
#include "DisplayWindow.h"

class HDROutputPlugin :
	public IHDROutputPlugin
{
private:
	std::mutex _lockApp;
	std::shared_ptr<App> _app;

	FnDebugLog _fnDebugLog;
	FnStateChangedCallback _fnStateChangedCallback;
	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11Texture2D> _sourceTexture;
	bool _asyncRender;

public:
	HDROutputPlugin();

private:
	virtual ~HDROutputPlugin();

public:
	virtual void Destroy() noexcept override;

	virtual void RunWindowProc(
		const PluginRect *initialWindowPosition,
		FnDebugLog fnDebugLog,
		FnStateChangedCallback fnStateChangedCallback,
		PluginRect *retClosedWindowPosition) noexcept override;

	virtual void CloseWindow() noexcept override;

	virtual PluginBool GetRequestHDR() noexcept override;
	virtual void SetRequestHDR(PluginBool flag) noexcept override;
	virtual PluginBool IsAvailableHDR() noexcept override;

	virtual PluginBool GetGammaCollect() override;
	virtual void SetGammaCollect(PluginBool flag) override;

	virtual PluginBool GetTopmost() override;
	virtual void SetTopmost(PluginBool flag) override;

	virtual void SetSourceTexture(IUnknown *src) noexcept override;
	virtual void UpdateSourceTextureDirect() noexcept override;

	virtual void RequestAsyncUpdateSourceTexture() noexcept override;

	virtual void SetD3D11Device(ID3D11Device *device) noexcept override;

	void RenderForUnityRenderingEvent() noexcept;

private:
	std::shared_ptr<App> GetApp()
	{
		std::lock_guard<std::mutex> lock(_lockApp);
		return _app;
	}

	void SetApp(std::shared_ptr<App> const& app)
	{
		std::lock_guard<std::mutex> lock(_lockApp);
		_app = app;
	}
};
