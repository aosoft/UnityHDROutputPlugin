using System;
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

		private delegate void FnRunWindowProc(IntPtr self, IntPtr wndParent, [In] ref PluginRect rect, IntPtr fnDebugLog, IntPtr fnStateChangedCallback, out PluginRect retLastRect);
		private delegate void FnRunWindowProcPtr(IntPtr self, IntPtr wndParent, IntPtr rect, IntPtr fnDebugLog, IntPtr fnStateChangedCallback, out PluginRect retLastRect);

		private delegate void FnSetSourceTexture(IntPtr self, IntPtr texture);
		private delegate IntPtr FnRequestAsyncUpdateSourceTexture(IntPtr self);

		private IntPtr _self;
		private FnAction _fnDestroy;
		private FnRunWindowProc _fnRunWindowProc;
		private FnRunWindowProcPtr _fnRunWindowProcPtr;
		private FnAction _fnCloseWindow;
		private FnGetFlag _fnGetRequestHDR;
		private FnSetFlag _fnSetRequestHDR;
		private FnGetFlag _fnIsAvailableHDR;
		private FnGetFlag _fnGetConvertColorSpace;
		private FnSetFlag _fnSetConvertColorSpace;
		private FnSetSourceTexture _fnSetSourceTexture;
		private FnAction _fnUpdateSourceTextureDirect;
		private FnRequestAsyncUpdateSourceTexture _fnRequestAsyncUpdateSourceTexture;

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
			_fnRunWindowProc = Marshal.GetDelegateForFunctionPointer<FnRunWindowProc>(buffer[2]);
			_fnRunWindowProcPtr = Marshal.GetDelegateForFunctionPointer<FnRunWindowProcPtr>(buffer[2]);
			_fnCloseWindow = Marshal.GetDelegateForFunctionPointer<FnAction>(buffer[3]);
			_fnGetRequestHDR = Marshal.GetDelegateForFunctionPointer<FnGetFlag>(buffer[4]);
			_fnSetRequestHDR = Marshal.GetDelegateForFunctionPointer<FnSetFlag>(buffer[5]);
			_fnIsAvailableHDR = Marshal.GetDelegateForFunctionPointer<FnGetFlag>(buffer[6]);
			_fnGetConvertColorSpace = Marshal.GetDelegateForFunctionPointer<FnGetFlag>(buffer[7]);
			_fnSetConvertColorSpace = Marshal.GetDelegateForFunctionPointer<FnSetFlag>(buffer[8]);
			_fnSetSourceTexture = Marshal.GetDelegateForFunctionPointer<FnSetSourceTexture>(buffer[9]);
			_fnUpdateSourceTextureDirect = Marshal.GetDelegateForFunctionPointer<FnAction>(buffer[10]);
			_fnRequestAsyncUpdateSourceTexture = Marshal.GetDelegateForFunctionPointer<FnRequestAsyncUpdateSourceTexture>(buffer[11]);
		}

		public void Dispose()
		{
			_fnDestroy?.Invoke(_self);
			_fnDestroy = null;
			_self = IntPtr.Zero;
		}

		public UnityEngine.RectInt RunWindowProc(
			IntPtr wndParent,
			UnityEngine.RectInt? initialPosition,
			FnDebugLog fnDebugLogFunc, FnStateChangedCallback fnStateChangedCallback)
		{
			PluginRect retLastRect;

			var pfnDebugLogFunc = fnDebugLogFunc != null ?
				Marshal.GetFunctionPointerForDelegate<FnDebugLog>(fnDebugLogFunc) :
				IntPtr.Zero;

			var pfnStateChangedCallback = fnStateChangedCallback != null ?
				Marshal.GetFunctionPointerForDelegate<FnStateChangedCallback>(fnStateChangedCallback) :
				IntPtr.Zero;

			if (initialPosition.HasValue)
			{
				PluginRect rect;
				rect.X = initialPosition.Value.x;
				rect.Y = initialPosition.Value.y;
				rect.Width = initialPosition.Value.width;
				rect.Height = initialPosition.Value.height;

				_fnRunWindowProc(_self, wndParent, ref rect, pfnDebugLogFunc, pfnStateChangedCallback, out retLastRect);
			}
			else
			{
				_fnRunWindowProcPtr(_self, wndParent, IntPtr.Zero, pfnDebugLogFunc, pfnStateChangedCallback, out retLastRect);
			}

			return new UnityEngine.RectInt(retLastRect.X, retLastRect.Y, retLastRect.Width, retLastRect.Height);
		}

		public void CloseWindow()
		{
			_fnCloseWindow(_self);
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

		public bool ConvertColorSpace
		{
			get
			{
				return _fnGetConvertColorSpace(_self).ToBool();
			}

			set
			{
				_fnSetConvertColorSpace(_self, value.ToPluginBool());
			}
		}

		public void SetSourceTexture(IntPtr texture)
		{
			_fnSetSourceTexture(_self, texture);
		}

		public void UpdateSourceTextureDirect()
		{
			_fnUpdateSourceTextureDirect(_self);
		}

		public void UpdateSourceTextureAsync()
		{
			_fnRequestAsyncUpdateSourceTexture(_self);
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

