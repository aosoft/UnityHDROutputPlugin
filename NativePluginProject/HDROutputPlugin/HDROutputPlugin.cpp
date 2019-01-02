﻿#include "stdafx.h"
#include "HDROutputPluginImpl.h"
#include <vector>

IUnityInterfaces *g_unityInterfaces = nullptr;
static std::vector<HDROutputPlugin *> g_plugins;

void UNITY_INTERFACE_API OnUnityRenderingEvent(int eventId)
{
	for (auto it = g_plugins.begin(); it != g_plugins.end(); it++)
	{
		(*it)->UpdateSourceTextureDirect();
	}
}

HDROutputPlugin::HDROutputPlugin() :
	_fnDebugLog(nullptr),
	_fnStateChangedCallback(nullptr),
	_asyncRender(false)
{
}

HDROutputPlugin::~HDROutputPlugin()
{
}

void HDROutputPlugin::Destroy() noexcept
{
	for (auto it = g_plugins.begin(); it != g_plugins.end(); it++)
	{
		if (this == *it)
		{
			g_plugins.erase(it);
			break;
		}
	}
	CloseWindow();

	while (true)
	{
		{
			std::lock_guard<std::mutex> lock(_lockApp);
			if (_app == nullptr)
			{
				break;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	delete this;
}

void HDROutputPlugin::RunWindowProc(
	const PluginRect *initialWindowPosition,
	FnDebugLog fnDebugLog,
	FnStateChangedCallback fnStateChangedCallback,
	PluginRect *retClosedWindowPosition) noexcept try
{
	auto device = _device;

	if (device == nullptr && g_unityInterfaces != nullptr)
	{
		auto p = g_unityInterfaces->Get<IUnityGraphicsD3D11>();
		if (p != nullptr)
		{
			device = p->GetDevice();
		}
	}

	SetApp(std::make_shared<App>());
	_app->Run(device, _sourceTexture, initialWindowPosition, fnDebugLog, fnStateChangedCallback, retClosedWindowPosition);
	SetApp(nullptr);
}
catch (const std::exception& e)
{
	SetApp(nullptr);
	ErrorLog(fnDebugLog, e);
}
catch (const _com_error& e)
{
	SetApp(nullptr);
	ErrorLog(fnDebugLog, e);
}

void HDROutputPlugin::CloseWindow() noexcept
{
	auto app = GetApp();
	if (app == nullptr)
	{
		return;
	}
	app->BeginInvoke([app]()
	{
		auto w = app->GetWindow().lock();
		if (w != nullptr)
		{
			w->DestroyWindow();
		}
	});
}

PluginBool HDROutputPlugin::GetRequestHDR() noexcept
{
	auto app = GetApp();
	auto w = app != nullptr ? app->GetWindow().lock() : nullptr;
	return ToPluginBool(w != nullptr && w->GetRequestHDR());
}

void HDROutputPlugin::SetRequestHDR(PluginBool flag) noexcept
{
	auto app = GetApp();
	auto w = app != nullptr ? app->GetWindow().lock() : nullptr;
	if (w != nullptr)
	{
		w->SetRequestHDR(FromPluginBool(flag));
	}
}

PluginBool HDROutputPlugin::IsAvailableHDR() noexcept
{
	auto app = GetApp();
	auto w = app != nullptr ? app->GetWindow().lock() : nullptr;
	return ToPluginBool(w != nullptr && w->IsAvailableHDR());
}

PluginBool HDROutputPlugin::GetGammaCollect()
{
	auto app = GetApp();
	auto w = app != nullptr ? app->GetWindow().lock() : nullptr;
	return ToPluginBool(w != nullptr && w->GetGammaCollect());
}

void HDROutputPlugin::SetGammaCollect(PluginBool flag)
{
	auto app = GetApp();
	auto w = app != nullptr ? app->GetWindow().lock() : nullptr;
	if (w != nullptr)
	{
		w->SetGammaCollect(FromPluginBool(flag));
	}
}

PluginBool HDROutputPlugin::GetTopmost()
{
	auto app = GetApp();
	auto w = app != nullptr ? app->GetWindow().lock() : nullptr;
	return ToPluginBool(w != nullptr && w->GetTopmost());
}

void HDROutputPlugin::SetTopmost(PluginBool flag)
{
	auto app = GetApp();
	if (app == nullptr)
	{
		return;
	}
	app->BeginInvoke([app, flag]()
	{
		auto w = app->GetWindow().lock();
		if (w != nullptr)
		{
			w->SetTopmost(FromPluginBool(flag));
		}
	});
}

void HDROutputPlugin::SetSourceTexture(IUnknown *src) noexcept try
{
	ComPtr<ID3D11Texture2D> texture;
	if (src != nullptr)
	{
		if (FAILED(src->QueryInterface(&texture)))
		{
			texture = nullptr;
		}
	}

	auto app = GetApp();
	auto w = app != nullptr ? app->GetWindow().lock() : nullptr;
	if (w != nullptr)
	{
		w->SetSourceTexture(texture);
		_sourceTexture = nullptr;
	}
	else
	{
		_sourceTexture = texture;
	}
}
catch (const std::exception& e)
{
	ErrorLog(_fnDebugLog, e);
}
catch (const _com_error& e)
{
	ErrorLog(_fnDebugLog, e);
}

void HDROutputPlugin::UpdateSourceTextureDirect() noexcept try
{
	auto app = GetApp();
	auto w = app != nullptr ? app->GetWindow().lock() : nullptr;
	if (w != nullptr)
	{
		w->UpdateSourceTexture();
	}
}
catch (const std::exception& e)
{
	ErrorLog(_fnDebugLog, e);
}
catch (const _com_error& e)
{
	ErrorLog(_fnDebugLog, e);
}

void HDROutputPlugin::RequestAsyncUpdateSourceTexture() noexcept
{
	_asyncRender = true;
}

void HDROutputPlugin::SetD3D11Device(ID3D11Device *device) noexcept
{
	_device = device;
}

void HDROutputPlugin::RenderForUnityRenderingEvent() noexcept
{
	if (_asyncRender)
	{
		UpdateSourceTextureDirect();
		_asyncRender = false;
	}
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
	void **buffer, int32_t bufferSize) noexcept try
{
	typedef void * void_ptr;

	static const void_ptr funcs[] =
	{
		Proxy<void>::Func<&HDROutputPlugin::Destroy>,
		Proxy<void, const PluginRect *, FnDebugLog, FnStateChangedCallback, PluginRect *>::Func<&HDROutputPlugin::RunWindowProc>,
		Proxy<void>::Func<&HDROutputPlugin::CloseWindow>,
		Proxy<PluginBool>::Func<&HDROutputPlugin::GetRequestHDR>,
		Proxy<void, PluginBool>::Func<&HDROutputPlugin::SetRequestHDR>,
		Proxy<PluginBool>::Func<&HDROutputPlugin::IsAvailableHDR>,
		Proxy<PluginBool>::Func<&HDROutputPlugin::GetGammaCollect>,
		Proxy<void, PluginBool>::Func<&HDROutputPlugin::SetGammaCollect>,
		Proxy<PluginBool>::Func<&HDROutputPlugin::GetTopmost>,
		Proxy<void, PluginBool>::Func<&HDROutputPlugin::SetTopmost>,
		Proxy<void, IUnknown *>::Func<&HDROutputPlugin::SetSourceTexture>,
		Proxy<void>::Func<&HDROutputPlugin::UpdateSourceTextureDirect>,
		Proxy<void>::Func<&HDROutputPlugin::RequestAsyncUpdateSourceTexture>,
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

	auto plugin = new HDROutputPlugin();
	buffer[0] = plugin;
	int32_t index = 1;
	for (auto p : funcs)
	{
		buffer[index] = p;
		index++;
	}

	g_plugins.push_back(plugin);

	return requiredSize;
}
catch (const std::exception&)
{
	return 0;
}

void UNITY_INTERFACE_EXPORT *UNITY_INTERFACE_API GetUnityRenderingEvent() noexcept
{
	return OnUnityRenderingEvent;
}

void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces) noexcept
{
	g_unityInterfaces = unityInterfaces;
}

void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload() noexcept
{
	g_unityInterfaces = nullptr;
}

#ifdef __cplusplus
}
#endif



