// UnityInterfaceManager.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

static IUnityInterfaces *g_unityInterfaces = nullptr;
static UnityRenderingEvent g_unityRenderingEvent = nullptr;

IUnityInterfaces UNITY_INTERFACE_EXPORT *UNITY_INTERFACE_API GetUnityInterface()
{
	return g_unityInterfaces;
}

void UNITY_INTERFACE_API OnUnityRenderingEvent(int eventId)
{
	if (g_unityRenderingEvent != nullptr)
	{
		(*g_unityRenderingEvent)(eventId);
	}
}

UnityRenderingEvent UNITY_INTERFACE_EXPORT GetProxyUnityRenderingEvent()
{
	return OnUnityRenderingEvent;
}

void UNITY_INTERFACE_EXPORT SetUnityRenderingEvent(UnityRenderingEvent fn)
{
	g_unityRenderingEvent = fn;
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



