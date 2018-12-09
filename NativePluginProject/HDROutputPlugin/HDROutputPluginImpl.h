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

public:
	HDROutputPlugin();
	~HDROutputPlugin();

	virtual void Destroy() override;
	virtual void SetDebugLogFunc(FnDebugLog fnDebugLog) override;
	virtual void CreateDisplayWindow() override;
	virtual PluginBool IsAvailableDisplayWindow() override;

private:
	void ExceptionHandler(const std::exception& e);
};
