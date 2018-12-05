#pragma once

#include <stdint.h>
#include <memory>

#include "DisplayWindow.h"

enum class PluginBool : int32_t
{
	False = 0,
	True
};

class HDROutputPlugin
{
private:
	std::weak_ptr<DisplayWindow> _window;

public:
	HDROutputPlugin();
	~HDROutputPlugin();

	void Destroy();
	void CreateDisplayWindow();
	PluginBool IsAvailableDisplayWindow();
};
