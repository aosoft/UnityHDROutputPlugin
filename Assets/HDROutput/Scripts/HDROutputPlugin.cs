using System;
using System.Runtime.InteropServices;

namespace HDROutput
{
	public delegate int FnCreateHDROutputPluginInstance(IntPtr[] buffer, int bufferSize);

	public class HDROutputPlugin : IDisposable
	{
		private delegate void FnAction(IntPtr self);
		private delegate int FnIsAvailableDisplayWindow(IntPtr self);

		private IntPtr _self;
		private FnAction _fnDestroy;
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
			_fnCreateDisplayWindow = Marshal.GetDelegateForFunctionPointer<FnAction>(buffer[2]);
			_fnIsAvailableDisplayWindow = Marshal.GetDelegateForFunctionPointer<FnIsAvailableDisplayWindow>(buffer[3]);
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

		public bool IsAvailableDisplayWindow
		{
			get
			{
				return _fnIsAvailableDisplayWindow(_self) != 0;
			}
		}
	}
}

