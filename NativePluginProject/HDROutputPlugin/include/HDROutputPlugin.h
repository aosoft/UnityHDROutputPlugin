#pragma once

#include <stdint.h>
#include <d3d11.h>
#include <IUnityInterface.h>

enum class PluginBool : int32_t
{
	False = 0,
	True
};

constexpr PluginBool ToPluginBool(bool f)
{
	return f ? PluginBool::True : PluginBool::False;
}

constexpr bool FromPluginBool(PluginBool f)
{
	return f != PluginBool::False;
}


enum class PluginLogType : int32_t
{
	Information = 0,
	Warning,
	Error,
};

enum class PluginStateChanged : int32_t
{
	Unspecified = 0,
	WindowClosed,
	CurrentHDRState,
};

#pragma pack(push, 4)

struct PluginRect
{
	int32_t X;
	int32_t Y;
	int32_t Width;
	int32_t Height;
};

#pragma pack(pop)

using FnDebugLog = void(UNITY_INTERFACE_API *)(PluginLogType, const wchar_t *);
using FnStateChangedCallback = void(UNITY_INTERFACE_API *)(PluginStateChanged);

class IHDROutputPlugin
{
public:
	virtual void Destroy() = 0;
	virtual void SetCallbacks(FnDebugLog fnDebugLog, FnStateChangedCallback fnStateChangedCallback) = 0;
	virtual void CreateDisplayWindow(const PluginRect *initialPosition) = 0;
	virtual PluginBool IsAvailableDisplayWindow() = 0;
	virtual void GetWindowRect(PluginRect *retRect) = 0;

	virtual PluginBool GetRequestHDR() = 0;
	virtual void SetRequestHDR(PluginBool flag) = 0;
	virtual PluginBool IsAvailableHDR() = 0;
	virtual void Render(IUnknown *src) = 0;

	//	for TestApp
	virtual void SetD3D11Device(ID3D11Device *device) = 0;
};
