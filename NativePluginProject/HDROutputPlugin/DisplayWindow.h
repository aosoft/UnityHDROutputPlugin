#pragma once

#include <atlwin.h>
#include <memory>

class DisplayWindow :
	public ATL::CWindowImpl<DisplayWindow>,
	public std::enable_shared_from_this<DisplayWindow>
{
public:
	DECLARE_WND_CLASS(L"UnityHDROutput/DisplayWindow");

private:
	std::shared_ptr<DisplayWindow> _this;

public:
	BEGIN_MSG_MAP(DisplayWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_MOVE, OnMove)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

	virtual void OnFinalMessage(_In_ HWND /*hWnd*/) override;

	LRESULT OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMove(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

};
