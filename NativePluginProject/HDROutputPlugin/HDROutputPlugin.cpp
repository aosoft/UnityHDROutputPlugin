#include "stdafx.h"
#include "HDROutputPlugin.h"

HDROutputPlugin::HDROutputPlugin()
{
}

HDROutputPlugin::~HDROutputPlugin()
{
	auto w = _window.lock();
	if (w != nullptr)
	{
		w->DestroyWindow();
	}
}

void HDROutputPlugin::Destroy()
{
	delete this;
}

void HDROutputPlugin::CreateDisplayWindow()
{
	auto w = std::make_shared<DisplayWindow>();
	if (w->Create(nullptr) == nullptr)
	{
		return;
	}

	_window = w;
}

PluginBool HDROutputPlugin::IsAvailableDisplayWindow()
{
	return _window.expired() ? PluginBool::False : PluginBool::True;
}


