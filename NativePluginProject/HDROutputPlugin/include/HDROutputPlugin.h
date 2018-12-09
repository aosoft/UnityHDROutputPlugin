#pragma once

#include "common.h"

#include "DisplayWindow.h"

enum class PluginBool : int32_t
{
	False = 0,
	True
};

class IHDROutputPlugin
{
public:
	virtual void Destroy() = 0;
	virtual void CreateDisplayWindow() = 0;
	virtual PluginBool IsAvailableDisplayWindow() = 0;
};

class HDROutputPlugin :
	public IHDROutputPlugin
{
private:
	std::weak_ptr<DisplayWindow> _window;

public:
	HDROutputPlugin();
	~HDROutputPlugin();

	virtual void Destroy() override;
	virtual void CreateDisplayWindow() override;
	virtual PluginBool IsAvailableDisplayWindow() override;
};
