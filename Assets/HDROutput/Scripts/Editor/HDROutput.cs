#define DYNAMIC_DLL_LOAD

using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace HDROutput
{
	public class HDROutput : MonoBehaviour
	{
#if DYNAMIC_DLL_LOAD
		private DllManager _dllManager = null;
#endif

		private HDROutputPlugin _plugin = null;

		private void Awake()
		{
#if DYNAMIC_DLL_LOAD
			_dllManager = new DllManager("Assets/HDROutput/Plugins/x86_64/HDROutputPlugin.dll");
			_plugin = new HDROutputPlugin(_dllManager.GetDelegate<FnCreateHDROutputPluginInstance>("CreateHDROutputPluginInstance"));
#endif

		}

		private void OnDestroy()
		{
			_plugin?.Dispose();
			_plugin = null;

#if DYNAMIC_DLL_LOAD
			_dllManager?.Dispose();
			_dllManager = null;
#endif
		}

		// Use this for initialization
		void Start()
		{
			_plugin?.SetDebugLogFunc(
				(logtype, msg) =>
				{
					switch (logtype)
					{
						case PluginLogType.Information:
							Debug.Log(msg);
							break;

						case PluginLogType.Warning:
							Debug.LogWarning(msg);
							break;

						case PluginLogType.Error:
							Debug.LogError(msg);
							break;
					}
				});
			_plugin?.CreateDisplayWindow(null);
		}

		// Update is called once per frame
		void Update()
		{

		}

	}
}