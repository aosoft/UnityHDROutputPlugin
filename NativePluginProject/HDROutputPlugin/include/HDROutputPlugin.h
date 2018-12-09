#pragma once

#include <stdint.h>

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
