#include <Windows.h>
#include <HDROutputPlugin.h>
#include <vector>
#include <comdef.h>

#pragma comment(lib, "HDROutputPlugin.lib")
#pragma comment(lib, "d3d11.lib")

extern "C" int32_t __declspec(dllimport) UNITY_INTERFACE_API CreateHDROutputPluginInstance(
	void **buffer, int32_t bufferSize);

template<class Intf>
using ComPtr = _com_ptr_t<_com_IIID<Intf, &__uuidof(Intf)>>;

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

void UNITY_INTERFACE_API DebugLog(PluginLogType logtype, const wchar_t *msg)
{
	::MessageBoxW(nullptr, msg, L"DebugLog", MB_OK | MB_ICONERROR | MB_TOPMOST);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hInst, LPWSTR pszCommandLine, int)
{
	CCoInitialize coinit;
	HRESULT hr = S_OK;

	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> dc;
	D3D_FEATURE_LEVEL feature;

	hr = D3D11CreateDevice(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_SINGLETHREADED,
		nullptr, 0,
		D3D11_SDK_VERSION,
		&device,
		&feature,
		&dc);
	if (FAILED(hr))
	{
		return hr;
	}

	int32_t count = CreateHDROutputPluginInstance(nullptr, 0);
	std::vector<void *> v;
	v.resize(count);
	count = CreateHDROutputPluginInstance(&v[0], count);

	IHDROutputPlugin *plugin = reinterpret_cast<IHDROutputPlugin *>(v[0]);


	plugin->SetD3D11Device(device);
	plugin->SetDebugLogFunc(DebugLog);
	plugin->CreateDisplayWindow();

	MSG msg;
	while (plugin->IsAvailableDisplayWindow() == PluginBool::True)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			plugin->Render(nullptr);
		}
	}

	plugin->Destroy();

	return 0;
}