#pragma once

#include "common.h"
#include <HDROutputPlugin.h>

#include "App.h"
#include "DisplayWindow.h"

class HDROutputPlugin :
	public IHDROutputPlugin
{
private:
	std::shared_ptr<App> _app;
	std::weak_ptr<DisplayWindow> _window;

	FnDebugLog _fnDebugLog;
	FnStateChangedCallback _fnStateChangedCallback;
	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11Texture2D> _sourceTexture;
	bool _asyncRender;

public:
	HDROutputPlugin();
	~HDROutputPlugin();

	virtual void Destroy() noexcept override;

	virtual void RunWindowProc(
		const PluginRect *initialWindowPosition,
		FnDebugLog fnDebugLog,
		FnStateChangedCallback fnStateChangedCallback,
		PluginRect *retClosedWindowPosition) noexcept override;

	virtual PluginBool GetRequestHDR() noexcept override;
	virtual void SetRequestHDR(PluginBool flag) noexcept override;
	virtual PluginBool IsAvailableHDR() noexcept override;
	virtual void SetSourceTexture(IUnknown *src) noexcept override;
	virtual void UpdateSourceTextureDirect() noexcept override;

	virtual void RequestAsyncUpdateSourceTexture() noexcept override;

	virtual void SetD3D11Device(ID3D11Device *device) noexcept override;

	void RenderForUnityRenderingEvent() noexcept;
};
