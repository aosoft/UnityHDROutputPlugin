#pragma once

#include <stdint.h>
#include <d3d11.h>
#include <IUnityInterface.h>
#include <IUnityGraphics.h>

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
	CurrentHDRStateChanged,
};

enum class PluginColorSpace : int32_t
{
	sRGB = 0,
	BT2100_PQ,
	BT709_Linear
};

#pragma pack(push, 4)

struct PluginRect
{
	int32_t X;
	int32_t Y;
	int32_t Width;
	int32_t Height;

	inline void UpdateFromHWnd(HWND hwnd)
	{
		RECT rect;
		if (IsWindow(hwnd) && GetWindowRect(hwnd, &rect))
		{
			X = rect.left;
			Y = rect.top;
			Width = rect.right - rect.left;
			Height = rect.bottom - rect.top;
		}
		else
		{
			*this = {};
		}
	}

	inline void CopyToWindowRect(RECT& rect) const
	{
		rect.left = X;
		rect.top = Y;
		rect.right = X + Width;
		rect.bottom = Y + Height;
	}
};

#pragma pack(pop)

using FnDebugLog = void(UNITY_INTERFACE_API *)(PluginLogType, const wchar_t *);
using FnStateChangedCallback = void(UNITY_INTERFACE_API *)(PluginStateChanged);

class IHDROutputPlugin
{
public:
	virtual void Destroy() noexcept = 0;

	virtual void RunWindowProc(
		HWND wndParent,
		const PluginRect *initialWindowPosition,
		FnDebugLog fnDebugLog,
		FnStateChangedCallback fnStateChangedCallback,
		PluginRect *retClosedWindowPosition) noexcept = 0;

	virtual void CloseWindow() noexcept = 0;

	virtual PluginColorSpace GetRequestColorSpace() noexcept = 0;
	virtual void SetRequestColorSpace(PluginColorSpace colorSpace) noexcept = 0;
	virtual PluginColorSpace GetActiveColorSpace() noexcept = 0;

	virtual PluginBool GetConvertColorSpace() = 0;
	virtual void SetConvertColorSpace(PluginBool flag) = 0;

	virtual float GetRelativeEV() noexcept = 0;
	virtual void SetRelativeEV(float value) noexcept = 0;

	virtual void SetSourceTexture(IUnknown *src) noexcept = 0;
	virtual void UpdateSourceTextureDirect() noexcept = 0;

	//	set flag only. require to invoke "GL.IssuePluginEvent".
	virtual void RequestAsyncUpdateSourceTexture() noexcept = 0;

	//	for TestApp
	virtual void SetD3D11Device(ID3D11Device *device) noexcept = 0;
};
