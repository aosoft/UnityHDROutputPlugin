// UnityInterfaceManager.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

IUnityInterfaces *g_unityInterfaces = nullptr;

IUnityInterfaces UNITY_INTERFACE_EXPORT *UNITY_INTERFACE_API GetUnityInterface()
{
	return g_unityInterfaces;
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



