#include <Windows.h>
#include <HDROutputPlugin.h>
#include <vector>
#include <comdef.h>
#include <random>
#include <DirectXMath.h>

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

IHDROutputPlugin *g_plugin = nullptr;

void UNITY_INTERFACE_API DebugLog(PluginLogType logtype, const wchar_t *msg)
{
	::MessageBoxW(nullptr, msg, L"DebugLog", MB_OK | MB_ICONERROR | MB_TOPMOST);
}

void UNITY_INTERFACE_API StateChangedCallback(PluginStateChanged state)
{
}

ComPtr<ID3D11Texture2D> CreateSampleTexture(
	ComPtr<ID3D11Device> const& device, uint32_t width, uint32_t height)
{
	std::random_device rnd;
	std::mt19937 mt(rnd());

	ComPtr<ID3D11Texture2D> ret;
	std::vector<uint32_t> buf;
	buf.resize(width * height);

	for (auto& n : buf)
	{
		n = mt() | 0xff000000;
	}
	
	D3D11_SUBRESOURCE_DATA resource = { &buf[0], width * sizeof(uint32_t), 0 };

	if (FAILED(device->CreateTexture2D(
		&CD3D11_TEXTURE2D_DESC(
			DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1,
			D3D11_BIND_SHADER_RESOURCE,
			D3D11_USAGE_IMMUTABLE),
		&resource,
		&ret)))
	{
		ret = nullptr;
	}

	return ret;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hInst, LPWSTR pszCommandLine, int)
{
	CCoInitialize coinit;
	HRESULT hr = S_OK;

	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> dc;
	ComPtr<ID3D11Texture2D> texture;
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
		return 0;
	}

	texture = CreateSampleTexture(device, 1280, 720);
	if (texture == nullptr)
	{
		return 0;
	}

	int32_t count = CreateHDROutputPluginInstance(nullptr, 0);
	std::vector<void *> v;
	v.resize(count);
	count = CreateHDROutputPluginInstance(&v[0], count);

	g_plugin = reinterpret_cast<IHDROutputPlugin *>(v[0]);


	g_plugin->SetD3D11Device(device);

	g_plugin->SetSourceTexture(texture);
	g_plugin->SetRequestColorSpace(PluginColorSpace::BT2100_PQ);
	g_plugin->SetConvertColorSpace(PluginBool::True);

	PluginRect closed;
	g_plugin->RunWindowProc(nullptr, nullptr, DebugLog, StateChangedCallback, &closed);

	dc->ClearState();
	g_plugin->Destroy();

	texture = nullptr;
	dc = nullptr;

#ifdef _DEBUG
	ComPtr<ID3D11Debug> debug;
	if (SUCCEEDED(device->QueryInterface(&debug)))
	{
		device = nullptr;
		debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	}
#endif

	device = nullptr;

	return 0;
}