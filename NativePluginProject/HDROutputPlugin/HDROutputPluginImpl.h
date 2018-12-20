#pragma once

#include "common.h"
#include <HDROutputPlugin.h>
#include "DisplayWindow.h"

class HDROutputPlugin :
	public IHDROutputPlugin
{
private:
	std::weak_ptr<DisplayWindow> _window;
	FnDebugLog _fnDebugLog;
	FnStateChangedCallback _fnStateChangedCallback;
	ComPtr<ID3D11Device> _device;
	bool _asyncRender;

public:
	HDROutputPlugin();
	~HDROutputPlugin();

	virtual void Destroy() override;
	virtual void SetCallbacks(FnDebugLog fnDebugLog, FnStateChangedCallback fnStateChangedCallback) override;
	virtual void CreateDisplayWindow(const PluginRect *initialPosition) override;
	virtual PluginBool IsAvailableDisplayWindow() override;
	virtual void GetWindowRect(PluginRect *retRect) override;

	virtual PluginBool GetRequestHDR() override;
	virtual void SetRequestHDR(PluginBool flag) override;
	virtual PluginBool IsAvailableHDR() override;
	virtual void SetSourceTexture(IUnknown *src) override;
	virtual void RenderDirect() override;

	virtual void RequestAsyncRendering() override;

	virtual void SetD3D11Device(ID3D11Device *device) override;

	void RenderForUnityRenderingEvent();
};
