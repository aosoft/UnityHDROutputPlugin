#pragma once

#include <stdint.h>
#include <IUnityInterface.h>

enum class PluginBool : int32_t
{
	False = 0,
	True
};

enum class PluginLogType : int32_t
{
	Information = 0,
	Warning,
	Error,
};


using FnDebugLog = void(UNITY_INTERFACE_API *)(PluginLogType, const wchar_t *);

class IHDROutputPlugin
{
public:
	virtual void Destroy() = 0;
	virtual void SetDebugLogFunc(FnDebugLog fnDebugLog) = 0;
	virtual void CreateDisplayWindow() = 0;
	virtual PluginBool IsAvailableDisplayWindow() = 0;
};
