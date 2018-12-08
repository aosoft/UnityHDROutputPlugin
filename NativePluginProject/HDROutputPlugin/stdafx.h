// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、
// または、参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーからほとんど使用されていない部分を除外する
#define _ATL_NO_AUTOMATIC_NAMESPACE

#include <windows.h>

#include <stdint.h>
#include <exception>
#include <memory>

#include <d3d11.h>
#include <dxgi1_4.h>
#include <IUnityInterface.h>
#include <IUnityGraphicsD3D11.h>

#include <comdef.h>
#include <atlbase.h>
extern ATL::CAtlWinModule _Module;
#include <atlwin.h>


template<class Intf>
using ComPtr = _com_ptr_t<_com_IIID<Intf, &__uuidof(Intf)>>;


#include "common.h"
