﻿using System;
using System.Runtime.InteropServices;

namespace HDROutput
{
	public enum PluginLogType : int
	{
		Information = 0,
		Warning,
		Error,
	}

	public enum PluginStateChanged : int
	{
		Unspecified = 0,
		WindowSizeChanged,
		WindowClosing,
		CurrentHDRStateChanged,
	};

	internal enum PluginBool : int
	{
		False = 0,
		True
	};

	[StructLayout(LayoutKind.Sequential, Pack = 4)]
	internal struct PluginRect
	{
		public int X;
		public int Y;
		public int Width;
		public int Height;
	}

	public delegate int FnCreateHDROutputPluginInstance(IntPtr[] buffer, int bufferSize);

	[UnmanagedFunctionPointer(CallingConvention.Winapi, CharSet = CharSet.Unicode)]
	public delegate void FnDebugLog(PluginLogType logtype, string msg);

	[UnmanagedFunctionPointer(CallingConvention.Winapi, CharSet = CharSet.Unicode)]
	public delegate void FnStateChangedCallback(PluginStateChanged state);

	public class HDROutputPlugin : IDisposable
	{
		private delegate void FnAction(IntPtr self);
		private delegate PluginBool FnGetFlag(IntPtr self);
		private delegate void FnSetFlag(IntPtr self, PluginBool flag);

		private delegate void FnCreateDisplayWindow(IntPtr self, [In] ref PluginRect rect);
		private delegate void FnCreateDisplayWindowPtr(IntPtr self, IntPtr rect);
		private delegate void FnSetCallbacks(IntPtr self, IntPtr fnDebugLog, IntPtr fnStateChangedCallback);
		private delegate void FnGetWindowRect(IntPtr self, out PluginRect rect);
		private delegate void FnSetSourceTexture(IntPtr self, IntPtr texture);
		private delegate IntPtr FnRequestAsyncRendering(IntPtr self);

		private IntPtr _self;
		private FnAction _fnDestroy;
		private FnSetCallbacks _fnSetCallbacks;
		private FnCreateDisplayWindow _fnCreateDisplayWindow;
		private FnCreateDisplayWindowPtr _fnCreateDisplayWindowPtr;
		private FnGetFlag _fnIsAvailableDisplayWindow;
		private FnGetWindowRect _fnGetWindowRect;
		private FnGetFlag _fnGetRequestHDR;
		private FnSetFlag _fnSetRequestHDR;
		private FnGetFlag _fnIsAvailableHDR;
		private FnSetSourceTexture _fnSetSourceTexture;
		private FnAction _fnRenderDirect;
		private FnRequestAsyncRendering _fnRequestAsyncRendering;

		private IntPtr _fnUnityRenderingEvent;

		public HDROutputPlugin(FnCreateHDROutputPluginInstance fnCreateHDROutputPluginInstance, IntPtr fnUnityRenderingEvent)
		{
			_fnUnityRenderingEvent = fnUnityRenderingEvent;

			int bufferSize = fnCreateHDROutputPluginInstance(null, 0);
			var buffer = new IntPtr[bufferSize];
			if (fnCreateHDROutputPluginInstance(buffer, bufferSize) != bufferSize)
			{
				throw new Exception();
			}

			_self = buffer[0];
			_fnDestroy = Marshal.GetDelegateForFunctionPointer<FnAction>(buffer[1]);
			_fnSetCallbacks = Marshal.GetDelegateForFunctionPointer<FnSetCallbacks>(buffer[2]);
			_fnCreateDisplayWindow = Marshal.GetDelegateForFunctionPointer<FnCreateDisplayWindow>(buffer[3]);
			_fnCreateDisplayWindowPtr = Marshal.GetDelegateForFunctionPointer<FnCreateDisplayWindowPtr>(buffer[3]);
			_fnIsAvailableDisplayWindow = Marshal.GetDelegateForFunctionPointer<FnGetFlag>(buffer[4]);
			_fnGetWindowRect = Marshal.GetDelegateForFunctionPointer<FnGetWindowRect>(buffer[5]);
			_fnGetRequestHDR = Marshal.GetDelegateForFunctionPointer<FnGetFlag>(buffer[6]);
			_fnSetRequestHDR = Marshal.GetDelegateForFunctionPointer<FnSetFlag>(buffer[7]);
			_fnIsAvailableHDR = Marshal.GetDelegateForFunctionPointer<FnGetFlag>(buffer[8]);
			_fnSetSourceTexture = Marshal.GetDelegateForFunctionPointer<FnSetSourceTexture>(buffer[9]);
			_fnRenderDirect = Marshal.GetDelegateForFunctionPointer<FnAction>(buffer[10]);
			_fnRequestAsyncRendering = Marshal.GetDelegateForFunctionPointer<FnRequestAsyncRendering>(buffer[11]);
		}

		public void Dispose()
		{
			_fnDestroy?.Invoke(_self);
			_fnDestroy = null;
			_self = IntPtr.Zero;
		}

		public void CreateDisplayWindow(UnityEngine.RectInt? initialPosition)
		{
			if (initialPosition.HasValue)
			{
				PluginRect rect;
				rect.X = initialPosition.Value.x;
				rect.Y = initialPosition.Value.y;
				rect.Width = initialPosition.Value.width;
				rect.Height = initialPosition.Value.height;

				_fnCreateDisplayWindow(_self, ref rect);
			}
			else
			{
				_fnCreateDisplayWindowPtr(_self, IntPtr.Zero);
			}
		}

		public void SetCallbacks(FnDebugLog fnDebugLogFunc, FnStateChangedCallback fnStateChangedCallback)
		{
			var pfnDebugLogFunc = fnDebugLogFunc != null ?
				Marshal.GetFunctionPointerForDelegate<FnDebugLog>(fnDebugLogFunc) :
				IntPtr.Zero;

			var pfnStateChangedCallback = fnStateChangedCallback != null ?
				Marshal.GetFunctionPointerForDelegate<FnStateChangedCallback>(fnStateChangedCallback) :
				IntPtr.Zero;

			_fnSetCallbacks(_self, pfnDebugLogFunc, pfnStateChangedCallback);
		}

		public bool IsAvailableDisplayWindow
		{
			get
			{
				return _fnIsAvailableDisplayWindow(_self).ToBool();
			}
		}

		public UnityEngine.RectInt GetWindowRect()
		{
			PluginRect rect;
			_fnGetWindowRect(_self, out rect);

			return new UnityEngine.RectInt(rect.X, rect.Y, rect.Width, rect.Height);
		}

		public bool RequestHDR
		{
			get
			{
				return _fnGetRequestHDR(_self).ToBool();
			}

			set
			{
				_fnSetRequestHDR(_self, value.ToPluginBool());
			}
		}

		public bool IsAvailableHDR
		{
			get
			{
				return _fnIsAvailableHDR(_self).ToBool();
			}
		}

		public void SetSourceTexture(IntPtr texture)
		{
			_fnSetSourceTexture(_self, texture);
		}

		public void RenderDirect()
		{
			_fnRenderDirect(_self);
		}

		public void RenderAsync()
		{
			_fnRequestAsyncRendering(_self);
			UnityEngine.GL.IssuePluginEvent(_fnUnityRenderingEvent, 0);
		}
	}

	internal static class PluginBoolExtension
	{
		internal static PluginBool ToPluginBool(this bool f)
		{
			return f ? PluginBool.True : PluginBool.False;
		}

		internal static bool ToBool(this PluginBool f)
		{
			return f != PluginBool.False;
		}
	}
}

