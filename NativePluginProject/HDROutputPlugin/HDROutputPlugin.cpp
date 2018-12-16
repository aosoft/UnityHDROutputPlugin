﻿#include "stdafx.h"
#include "HDROutputPluginImpl.h"

IUnityInterfaces *g_unityInterfaces = nullptr;

HDROutputPlugin::HDROutputPlugin() :
	_fnDebugLog(nullptr),
	_fnStateChangedCallback(nullptr)
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

void HDROutputPlugin::SetCallbacks(FnDebugLog fnDebugLog, FnStateChangedCallback fnStateChangedCallback)
{
	_fnDebugLog = fnDebugLog;
	_fnStateChangedCallback = fnStateChangedCallback;
}

void HDROutputPlugin::CreateDisplayWindow(const PluginRect *initialPosition) try
{
	if (!_window.expired())
	{
		return;
	}

	auto device = _device;

	if (device == nullptr && g_unityInterfaces != nullptr)
	{
		auto p = g_unityInterfaces->Get<IUnityGraphicsD3D11>();
		if (p != nullptr)
		{
			device = p->GetDevice();
		}
	}

	auto w = DisplayWindow::CreateInstance(device, _fnDebugLog, nullptr);
	if (initialPosition != nullptr)
	{
		RECT rect2 =
		{
			initialPosition->X,
			initialPosition->Y,
			initialPosition->X + initialPosition->Width,
			initialPosition->Y + initialPosition->Height
		};
		if (::MonitorFromRect(&rect2, MONITOR_DEFAULTTONULL) != nullptr)
		{
			::SetWindowPos(
				w->m_hWnd, nullptr,
				initialPosition->X,
				initialPosition->Y,
				initialPosition->Width,
				initialPosition->Height,
				SWP_SHOWWINDOW);
		}
	}

	w->ShowWindow(SW_SHOW);
	_window = w;
}
catch (const std::exception& e)
{
	ErrorLog(_fnDebugLog, e);
}

PluginBool HDROutputPlugin::IsAvailableDisplayWindow()
{
	return ToPluginBool(!_window.expired());
}

void HDROutputPlugin::GetWindowRect(PluginRect *retRect) try
{
	if (retRect == nullptr)
	{
		return;
	}

	auto w = _window.lock();
	RECT rect;
	if (w != nullptr && w->GetWindowRect(&rect))
	{
		retRect->X = rect.left;
		retRect->Y = rect.top;
		retRect->Width = rect.right - rect.left;
		retRect->Height = rect.bottom - rect.top;
	}
	else
	{
		*retRect = PluginRect();
	}
}
catch (const std::exception& e)
{
	ErrorLog(_fnDebugLog, e);
}

PluginBool HDROutputPlugin::GetRequestHDR()
{
	auto w = _window.lock();
	return ToPluginBool(w != nullptr && w->GetRequestHDR());
}

void HDROutputPlugin::SetRequestHDR(PluginBool flag)
{
	auto w = _window.lock();
	if (w != nullptr)
	{
		w->SetRequestHDR(FromPluginBool(flag));
	}
}

PluginBool HDROutputPlugin::IsAvailableHDR()
{
	auto w = _window.lock();
	return ToPluginBool(w != nullptr && w->IsAvailableHDR());
}

void HDROutputPlugin::Render(IUnknown *src) try
{
	auto w = _window.lock();
	if (w != nullptr)
	{
		ComPtr<ID3D11Texture2D> texture;
		if (src != nullptr)
		{
			if (FAILED(src->QueryInterface(&texture)))
			{
				texture = nullptr;
			}
		}
		w->Render(texture);
	}
}
catch (const std::exception& e)
{
	ErrorLog(_fnDebugLog, e);
}

void HDROutputPlugin::SetD3D11Device(ID3D11Device *device)
{
	_device = device;
}




template<typename RetT, typename ... Args>
struct Proxy
{
	template<typename RetT(HDROutputPlugin::*func)(Args...)>
	static RetT UNITY_INTERFACE_API Func(HDROutputPlugin *self, Args... args)
	{
		return (self->*func)(args...);
	}
};


#ifdef __cplusplus
extern "C" {
#endif

int32_t UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API CreateHDROutputPluginInstance(
	void **buffer, int32_t bufferSize) try
{
	typedef void * void_ptr;

	static const void_ptr funcs[] =
	{
		Proxy<void>::Func<&HDROutputPlugin::Destroy>,
		Proxy<void, FnDebugLog, FnStateChangedCallback>::Func<&HDROutputPlugin::SetCallbacks>,
		Proxy<void, const PluginRect *>::Func<&HDROutputPlugin::CreateDisplayWindow>,
		Proxy<PluginBool>::Func<&HDROutputPlugin::IsAvailableDisplayWindow>,
		Proxy<void, PluginRect *>::Func<&HDROutputPlugin::GetWindowRect>,
		Proxy<PluginBool>::Func<&HDROutputPlugin::GetRequestHDR>,
		Proxy<void, PluginBool>::Func<&HDROutputPlugin::SetRequestHDR>,
		Proxy<PluginBool>::Func<&HDROutputPlugin::IsAvailableHDR>,
		Proxy<void, IUnknown *>::Func<&HDROutputPlugin::Render>,
	};

	static constexpr int32_t requiredSize = sizeof(funcs) / sizeof(void *) + 1;

	if (buffer == nullptr || bufferSize < 1)
	{
		return requiredSize;
	}

	if (bufferSize < requiredSize)
	{
		return 0;
	}

	buffer[0] = new HDROutputPlugin();
	int32_t index = 1;
	for (auto p : funcs)
	{
		buffer[index] = p;
		index++;
	}

	return requiredSize;
}
catch (const std::exception&)
{
	return 0;
}

void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
	g_unityInterfaces = unityInterfaces;
}

void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
	g_unityInterfaces = nullptr;
}

#ifdef __cplusplus
}
#endif



