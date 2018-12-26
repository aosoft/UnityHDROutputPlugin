#pragma once

#include <windows.h>
#include <comdef.h>
#include <d3d11.h>

#include <stdint.h>
#include <exception>
#include <array>
#include <memory>


template<class Intf>
using ComPtr = _com_ptr_t<_com_IIID<Intf, &__uuidof(Intf)>>;

class HRException :
	public std::exception
{
private:
	HRESULT _hr;

public:
	HRException(HRESULT hr) : _hr(hr)
	{
	}

	HRESULT GetResult() const noexcept
	{
		return _hr;
	}

	static void CheckHR(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw HRException(hr);
		}
	}

	template<typename T>
	static void CheckNull(const T *p)
	{
		if (p == nullptr)
		{
			throw HRException(E_POINTER);
		}
	}

	template<typename iid>
	static void CheckNull(_com_ptr_t<iid> const& p)
	{
		if (p == nullptr)
		{
			throw HRException(E_POINTER);
		}
	}

	template<typename T>
	static void CheckNull(std::unique_ptr<T> const& p)
	{
		if (p == nullptr)
		{
			throw HRException(E_POINTER);
		}
	}

	template<typename T>
	static void CheckNull(std::shared_ptr<T> const& p)
	{
		if (p == nullptr)
		{
			throw HRException(E_POINTER);
		}
	}
};

class CD3D11_SUBRESOURCE_DATA :
	public D3D11_SUBRESOURCE_DATA
{
public:
	CD3D11_SUBRESOURCE_DATA(
		const void *sysMem,
		UINT sysMemPitch,
		UINT sysMemSlicePitch)
	{
		pSysMem = sysMem;
		SysMemPitch = sysMemPitch;
		SysMemSlicePitch = sysMemSlicePitch;
	}

};