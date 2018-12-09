#include <Windows.h>
#include <HDROutputPlugin.h>
#include <vector>

#pragma comment(lib, "HDROutputPlugin.lib")

extern "C" int32_t __declspec(dllimport) UNITY_INTERFACE_API CreateHDROutputPluginInstance(
	void **buffer, int32_t bufferSize);


class CCoInitialize
{
public:
	CCoInitialize(DWORD dwCoInit = COINIT_MULTITHREADED)
	{
		::CoInitializeEx(NULL, dwCoInit);
	}

	~CCoInitialize()
	{
		::CoUninitialize();
	}
};

void UNITY_INTERFACE_API DebugLog(const wchar_t *msg)
{
	::MessageBoxW(nullptr, msg, L"DebugLog", MB_OK | MB_ICONERROR | MB_TOPMOST);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hInst, LPWSTR pszCommandLine, int)
{
	CCoInitialize coinit;

	int32_t count = CreateHDROutputPluginInstance(nullptr, 0);
	std::vector<void *> v;
	v.resize(count);
	count = CreateHDROutputPluginInstance(&v[0], count);

	IHDROutputPlugin *plugin = reinterpret_cast<IHDROutputPlugin *>(v[0]);

	plugin->SetDebugLogFunc(DebugLog);
	plugin->CreateDisplayWindow();

	MSG msg;
	while (plugin->IsAvailableDisplayWindow() == PluginBool::True)
	{
		if (GetMessage(&msg, 0, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	plugin->Destroy();

	return 0;
}