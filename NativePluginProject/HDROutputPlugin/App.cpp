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
	std::function<void(DisplayWindow *)> fnCreatedCallback,
	const PluginRect *initialWindowPosition,
	FnDebugLog fnDebugLog,
	FnStateChangedCallback fnStateChangedCallback,
	PluginRect *retClosedWindowPosition) noexcept
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

	/*{
		ComPtr<ID3D10Multithread> multithread;
		if (SUCCEEDED(device->QueryInterface(&multithread)))
		{
			multithread->SetMultithreadProtected(TRUE);
		}
	}*/

	auto w = DisplayWindow::CreateInstance(device, fnDebugLog, fnStateChangedCallback);
	auto sharedTexture = std::make_shared<SharedTexture>(device);

	if (initialWindowPosition != nullptr)
	{
		RECT rect2;
		initialWindowPosition->CopyToWindowRect(rect2);
		if (::MonitorFromRect(&rect2, MONITOR_DEFAULTTONULL) != nullptr)
		{
			::SetWindowPos(
				w->m_hWnd, nullptr,
				initialWindowPosition->X,
				initialWindowPosition->Y,
				initialWindowPosition->Width,
				initialWindowPosition->Height,
				SWP_SHOWWINDOW);
		}
	}

	if (fnCreatedCallback != nullptr)
	{
		fnCreatedCallback(w.get());
	}

	_window = w;

	w->ShowWindow(SW_SHOW);

	while (true)
	{
		try
		{
			MSG msg;
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				w->RenderIfUpdatedSourceTexture();
			}

			if (msg.message == WM_QUIT)
			{
				break;
			}

			std::function<void()> task;

			{
				std::lock_guard<std::mutex> lock(_lockTaskList);
				if (_queTask.size() > 0)
				{
					task = *_queTask.begin();
					_queTask.pop_front();
				}
				else
				{
					continue;
				}
			}

			task();
		}
		catch (const std::exception& e)
		{
			ErrorLog(fnDebugLog, e);
		}
		catch (const _com_error& e)
		{
			ErrorLog(fnDebugLog, e);
		}
	}

	if (retClosedWindowPosition != nullptr)
	{
		w->GetWindowPluginRect(*retClosedWindowPosition);
	}

	w = nullptr;

	dc->ClearState();
	dc = nullptr;

#ifdef _DEBUG
	ComPtr<ID3D11Debug> debug;
	if (SUCCEEDED(device->QueryInterface(&debug)))
	{
		device = nullptr;
		debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	}
#endif
}

void App::BeginInvoke(std::function<void()> task)
{
	std::lock_guard<std::mutex> lock(_lockTaskList);

	_queTask.push_back(task);
}
