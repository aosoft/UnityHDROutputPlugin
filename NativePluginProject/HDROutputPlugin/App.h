#pragma once

#include "common.h"
#include "DisplayWindow.h"

#include <thread>
#include <windows.h>


class App
{
private:
	std::weak_ptr<DisplayWindow> _window;

public:
	App();
	~App();

	std::weak_ptr<DisplayWindow> const& GetWindow()
	{
		return _window;
	}

	void Run(
		ComPtr<ID3D11Device> const& unityDevice,
		FnDebugLog fnDebugLog,
		FnStateChangedCallback fnStateChangedCallback);
};

