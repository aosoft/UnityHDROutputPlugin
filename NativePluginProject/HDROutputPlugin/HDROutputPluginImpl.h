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
	ComPtr<ID3D11Device> _device;

public:
	HDROutputPlugin();
	~HDROutputPlugin();

	virtual void Destroy() override;
	virtual void SetDebugLogFunc(FnDebugLog fnDebugLog) override;
	virtual void CreateDisplayWindow(const PluginRect *initialPosition) override;
	virtual PluginBool IsAvailableDisplayWindow() override;
	virtual void GetWindowRect(PluginRect *retRect) override;

	virtual PluginBool GetRequestHDR() override;
	virtual void SetRequestHDR(PluginBool flag) override;
	virtual PluginBool IsAvailableHDR() override;
	virtual void Render(IUnknown *src) override;

	virtual void SetD3D11Device(ID3D11Device *device) override;
};
