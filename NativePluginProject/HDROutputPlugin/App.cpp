#include "stdafx.h"
#include "App.h"
#pragma comment(lib, "d3d11.lib")

App::App()
{
}

App::~App()
{
}

void App::Run(
	ComPtr<ID3D11Device> const& unityDevice,
	FnDebugLog fnDebugLog,
	FnStateChangedCallback fnStateChangedCallback)
{
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

	CCoInitialize init;

	ComPtr<IDXGIDevice> dxgidevice;
	ComPtr<IDXGIAdapter> adapter;

	if (unityDevice == nullptr)
	{
		HRException::CheckHR(unityDevice->QueryInterface(&dxgidevice));
		HRException::CheckHR(dxgidevice->GetParent(IID_PPV_ARGS(&adapter)));
	}

	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> dc;
	ComPtr<ID3D11Texture2D> texture;
	D3D_FEATURE_LEVEL feature;

	HRException::CheckHR(D3D11CreateDevice(
		adapter, D3D_DRIVER_TYPE_HARDWARE, nullptr,
#if _DEBUG
		D3D11_CREATE_DEVICE_DEBUG |
#endif
		0,
		nullptr, 0,
		D3D11_SDK_VERSION,
		&device,
		&feature,
		&dc));

	_window = DisplayWindow::CreateInstance(device, fnDebugLog, fnStateChangedCallback);

	MSG msg;
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
