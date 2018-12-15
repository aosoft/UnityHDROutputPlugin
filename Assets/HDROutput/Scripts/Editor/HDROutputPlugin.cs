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

	public class HDROutputPlugin : IDisposable
	{
		private delegate void FnAction(IntPtr self);
		private delegate PluginBool FnGetFlag(IntPtr self);
		private delegate void FnSetFlag(IntPtr self, PluginBool flag);

		private delegate void FnCreateDisplayWindow(IntPtr self, [In] ref PluginRect rect);
		private delegate void FnCreateDisplayWindowPtr(IntPtr self, IntPtr rect);
		private delegate void FnSetDebugLogFunc(IntPtr self, IntPtr fnDebugLog);
		private delegate void FnGetWindowRect(IntPtr self, out PluginRect rect);
		private delegate void FnRender(IntPtr self, IntPtr texture);

		private IntPtr _self;
		private FnAction _fnDestroy;
		private FnSetDebugLogFunc _fnSetDebugLogFunc;
		private FnCreateDisplayWindow _fnCreateDisplayWindow;
		private FnCreateDisplayWindowPtr _fnCreateDisplayWindowPtr;
		private FnGetFlag _fnIsAvailableDisplayWindow;
		private FnGetWindowRect _fnGetWindowRect;
		private FnGetFlag _fnGetRequestHDR;
		private FnSetFlag _fnSetRequestHDR;
		private FnGetFlag _fnIsAvailableHDR;
		private FnRender _fnRender;

		public HDROutputPlugin(FnCreateHDROutputPluginInstance fnCreateHDROutputPluginInstance)
		{
			int bufferSize = fnCreateHDROutputPluginInstance(null, 0);
			var buffer = new IntPtr[bufferSize];
			if (fnCreateHDROutputPluginInstance(buffer, bufferSize) != bufferSize)
			{
				throw new Exception();
			}

			_self = buffer[0];
			_fnDestroy = Marshal.GetDelegateForFunctionPointer<FnAction>(buffer[1]);
			_fnSetDebugLogFunc = Marshal.GetDelegateForFunctionPointer<FnSetDebugLogFunc>(buffer[2]);
			_fnCreateDisplayWindow = Marshal.GetDelegateForFunctionPointer<FnCreateDisplayWindow>(buffer[3]);
			_fnCreateDisplayWindowPtr = Marshal.GetDelegateForFunctionPointer<FnCreateDisplayWindowPtr>(buffer[3]);
			_fnIsAvailableDisplayWindow = Marshal.GetDelegateForFunctionPointer<FnGetFlag>(buffer[4]);
			_fnGetWindowRect = Marshal.GetDelegateForFunctionPointer<FnGetWindowRect>(buffer[5]);
			_fnGetRequestHDR = Marshal.GetDelegateForFunctionPointer<FnGetFlag>(buffer[6]);
			_fnSetRequestHDR = Marshal.GetDelegateForFunctionPointer<FnSetFlag>(buffer[7]);
			_fnIsAvailableHDR = Marshal.GetDelegateForFunctionPointer<FnGetFlag>(buffer[8]);
			_fnRender = Marshal.GetDelegateForFunctionPointer<FnRender>(buffer[9]);
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

		public void SetDebugLogFunc(FnDebugLog fnDebugLogFunc)
		{
			if (fnDebugLogFunc != null)
			{
				_fnSetDebugLogFunc(_self, Marshal.GetFunctionPointerForDelegate<FnDebugLog>(fnDebugLogFunc));
			}
			else
			{
				_fnSetDebugLogFunc(_self, IntPtr.Zero);
			}
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

		public void Render(IntPtr texture)
		{
			_fnRender(_self, texture);
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

