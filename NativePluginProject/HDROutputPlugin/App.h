#pragma once

#include "common.h"
#include "DisplayWindow.h"
#include "SharedTexture.h"

#include <thread>
#include <functional>
#include <deque>

#include <windows.h>

class App
{
private:
	std::weak_ptr<DisplayWindow> _window;

	std::mutex _lockTaskList;
	std::deque<std::function<void()>> _queTask;

public:
	App();
	~App();

	std::weak_ptr<DisplayWindow> const& GetWindow()
	{
		return _window;
	}

	void Run(
		ComPtr<ID3D11Device> const& unityDevice,
		ComPtr<ID3D11Texture2D> const& sourceTexture,
		const PluginRect *initialWindowPosition,
		FnDebugLog fnDebugLog,
		FnStateChangedCallback fnStateChangedCallback,
		PluginRect *retClosedWindowPosition) noexcept;

	void BeginInvoke(std::function<void()> task);
};

