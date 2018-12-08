#pragma once

#include <windows.h>
#include <exception>

class HRException :
	public std::exception
{
private:
	HRESULT _hr;

public:
	HRException(HRESULT hr) : _hr(hr)
	{
	}

	HRESULT GetResult() const
	{
		return _hr;
	}

	static void ThrowExceptionForHR(HRESULT hr)
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
