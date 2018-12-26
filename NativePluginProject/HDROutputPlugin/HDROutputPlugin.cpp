#include "stdafx.h"
#include "HDROutputPluginImpl.h"
#include <vector>

IUnityInterfaces *g_unityInterfaces = nullptr;
static std::vector<HDROutputPlugin *> g_plugins;

void UNITY_INTERFACE_API OnUnityRenderingEvent(int eventId)
{
	for (auto it = g_plugins.begin(); it != g_plugins.end(); it++)
	{
		(*it)->RenderDirect();
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
	auto w = _window.lock();
	if (w != nullptr)
	{
		w->DestroyWindow();
	}
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
	delete this;
}

void HDROutputPlugin::SetCallbacks(FnDebugLog fnDebugLog, FnStateChangedCallback fnStateChangedCallback) noexcept
{
	_fnDebugLog = fnDebugLog;
	_fnStateChangedCallback = fnStateChangedCallback;
}

void HDROutputPlugin::CreateDisplayWindow(const PluginRect *initialPosition) noexcept try
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

	auto w = DisplayWindow::CreateInstance(device, _fnDebugLog, _fnStateChangedCallback);
	if (initialPosition != nullptr)
	{
		RECT rect2;
		initialPosition->CopyToWindowRect(rect2);
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
catch (const _com_error& e)
{
	ErrorLog(_fnDebugLog, e);
}

PluginBool HDROutputPlugin::IsAvailableDisplayWindow() noexcept
{
	return ToPluginBool(!_window.expired());
}

void HDROutputPlugin::GetWindowRect(PluginRect *retRect) noexcept try
{
	if (retRect == nullptr)
	{
		return;
	}

	auto w = _window.lock();
	if (w != nullptr)
	{
		w->GetWindowPluginRect(*retRect);
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

	_app = std::make_shared<App>();
	_app->Run(device, initialWindowPosition, fnDebugLog, fnStateChangedCallback, retClosedWindowPosition);
	_app = nullptr;
}
catch (const std::exception& e)
{
	ErrorLog(fnDebugLog, e);
}
catch (const _com_error& e)
{
	ErrorLog(fnDebugLog, e);
}


PluginBool HDROutputPlugin::GetRequestHDR() noexcept
{
	auto w = _window.lock();
	return ToPluginBool(w != nullptr && w->GetRequestHDR());
}

void HDROutputPlugin::SetRequestHDR(PluginBool flag) noexcept
{
	auto w = _window.lock();
	if (w != nullptr)
	{
		w->SetRequestHDR(FromPluginBool(flag));
	}
}

PluginBool HDROutputPlugin::IsAvailableHDR() noexcept
{
	auto w = _window.lock();
	return ToPluginBool(w != nullptr && w->IsAvailableHDR());
}

void HDROutputPlugin::SetSourceTexture(IUnknown *src) noexcept try
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
		w->SetSourceTexture(texture);
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

void HDROutputPlugin::RenderDirect() noexcept try
{
	auto w = _window.lock();
	if (w != nullptr)
	{
		w->Render();
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

void HDROutputPlugin::RequestAsyncRendering() noexcept
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
		RenderDirect();
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
		Proxy<void, FnDebugLog, FnStateChangedCallback>::Func<&HDROutputPlugin::SetCallbacks>,
		Proxy<void, const PluginRect *>::Func<&HDROutputPlugin::CreateDisplayWindow>,
		Proxy<PluginBool>::Func<&HDROutputPlugin::IsAvailableDisplayWindow>,
		Proxy<void, PluginRect *>::Func<&HDROutputPlugin::GetWindowRect>,
		Proxy<PluginBool>::Func<&HDROutputPlugin::GetRequestHDR>,
		Proxy<void, PluginBool>::Func<&HDROutputPlugin::SetRequestHDR>,
		Proxy<PluginBool>::Func<&HDROutputPlugin::IsAvailableHDR>,
		Proxy<void, IUnknown *>::Func<&HDROutputPlugin::SetSourceTexture>,
		Proxy<void>::Func<&HDROutputPlugin::RenderDirect>,
		Proxy<void>::Func<&HDROutputPlugin::RequestAsyncRendering>,
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



