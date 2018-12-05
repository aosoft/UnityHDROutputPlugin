#include "stdafx.h"
#include "DisplayWindow.h"

void DisplayWindow::OnFinalMessage(_In_ HWND /*hWnd*/)
{
	_this = nullptr;
}


LRESULT DisplayWindow::OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	_this = shared_from_this();
	bHandled = TRUE;
	return 0;
}

LRESULT DisplayWindow::OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	return 0;
}

LRESULT DisplayWindow::OnMove(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	return 0;
}

LRESULT DisplayWindow::OnSize(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	return 0;
}

