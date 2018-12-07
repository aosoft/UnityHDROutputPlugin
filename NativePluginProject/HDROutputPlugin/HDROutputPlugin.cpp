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

IUnityInterfaces *g_unityInterfaces = nullptr;

int32_t UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API CreateHDROutputPluginInstance(
	void **buffer, int32_t bufferSize) try
{
	typedef void * void_ptr;

	static const void_ptr funcs[] =
	{
		Proxy<void>::Func<&HDROutputPlugin::Destroy>,
		Proxy<void>::Func<&HDROutputPlugin::CreateDisplayWindow>,
		Proxy<PluginBool>::Func<&HDROutputPlugin::IsAvailableDisplayWindow>,
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



