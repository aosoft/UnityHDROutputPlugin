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

	public delegate int FnCreateHDROutputPluginInstance(IntPtr[] buffer, int bufferSize);

	[UnmanagedFunctionPointer(CallingConvention.Winapi, CharSet = CharSet.Unicode)]
	public delegate void FnDebugLog(PluginLogType logtype, string msg);

	public class HDROutputPlugin : IDisposable
	{
		private delegate void FnAction(IntPtr self);
		private delegate void FnSetDebugLogFunc(IntPtr self, IntPtr fnDebugLog);
		private delegate int FnIsAvailableDisplayWindow(IntPtr self);

		private IntPtr _self;
		private FnAction _fnDestroy;
		private FnSetDebugLogFunc _fnSetDebugLogFunc;
		private FnAction _fnCreateDisplayWindow;
		private FnIsAvailableDisplayWindow _fnIsAvailableDisplayWindow;

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
			_fnCreateDisplayWindow = Marshal.GetDelegateForFunctionPointer<FnAction>(buffer[3]);
			_fnIsAvailableDisplayWindow = Marshal.GetDelegateForFunctionPointer<FnIsAvailableDisplayWindow>(buffer[4]);
		}

		public void Dispose()
		{
			_fnDestroy?.Invoke(_self);
			_fnDestroy = null;
			_self = IntPtr.Zero;
		}

		public void CreateDisplayWindow()
		{
			_fnCreateDisplayWindow(_self);
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
				return _fnIsAvailableDisplayWindow(_self) != 0;
			}
		}
	}
}

