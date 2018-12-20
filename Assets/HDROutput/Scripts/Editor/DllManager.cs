using System;
using System.Runtime.InteropServices;

namespace HDROutput
{
	public class DllManager : IDisposable
	{
		private IntPtr _dll = IntPtr.Zero;

		delegate void FnUnityPluginLoad(IntPtr unityInterfaces);
		delegate void FnUnityPluginUnload();
		delegate IntPtr FnGetUnityRenderingEvent();

		public DllManager(string dllpath)
		{
			_dll = LoadLibrary(dllpath);

			if (_dll != IntPtr.Zero)
			{
				GetDelegate<FnUnityPluginLoad>("UnityPluginLoad")?.Invoke(GetUnityInterface());
				SetUnityRenderingEvent((GetDelegate<FnGetUnityRenderingEvent>("GetUnityRenderingEvent")?.Invoke()).GetValueOrDefault());
			}
		}

		public void Dispose()
		{
			if (_dll != IntPtr.Zero)
			{
				SetUnityRenderingEvent(IntPtr.Zero);
				GetDelegate<FnUnityPluginUnload>("UnityPluginUnload")?.Invoke();
				FreeLibrary(_dll);
				_dll = IntPtr.Zero;
			}
		}

		public TDelegate GetDelegate<TDelegate>(string procname)
		{
			if (_dll != IntPtr.Zero)
			{
				return Marshal.GetDelegateForFunctionPointer<TDelegate>(GetProcAddress(_dll, procname));
			}
			return default(TDelegate);
		}

		[DllImport("UnityInterfaceManager")]
		private static extern IntPtr GetUnityInterface();

		[DllImport("UnityInterfaceManager")]
		public static extern IntPtr GetProxyUnityRenderingEvent();

		[DllImport("UnityInterfaceManager")]
		private static extern void SetUnityRenderingEvent(IntPtr fn);

		[DllImport("kernel32", SetLastError = true, CharSet = CharSet.Unicode)]
		private static extern IntPtr LoadLibrary(string lpFileName);

		[DllImport("kernel32", SetLastError = true)]
		[return: MarshalAs(UnmanagedType.Bool)]
		private static extern bool FreeLibrary(IntPtr hModule);

		[DllImport("kernel32", CharSet = CharSet.Ansi, SetLastError = true)]
		private static extern IntPtr GetProcAddress(IntPtr hModule, string procName);
	}
}
