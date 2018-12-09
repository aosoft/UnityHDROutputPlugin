#include <atlbase.h>
#include <HDROutputPlugin.h>
#include <vector>

#pragma comment(lib, "HDROutputPlugin.lib")

extern "C" int32_t __declspec(dllimport) STDAPICALLTYPE CreateHDROutputPluginInstance(
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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hInst, LPWSTR pszCommandLine, int)
{
	CCoInitialize coinit;

	int32_t count = CreateHDROutputPluginInstance(nullptr, 0);
	std::vector<void *> v;
	v.resize(count);
	count = CreateHDROutputPluginInstance(&v[0], count);

	IHDROutputPlugin *plugin = reinterpret_cast<IHDROutputPlugin *>(v[0]);

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

	return 0;
}