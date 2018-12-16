#define DYNAMIC_DLL_LOAD

using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace HDROutput
{
	public class HDROutput : EditorWindow
	{
#if DYNAMIC_DLL_LOAD
		private DllManager _dllManager = null;
#endif

		private HDROutputPlugin _plugin = null;
		private bool _requestCreateWindow = false;

		[SerializeField]
		private RenderTexture _renderTexture;

		[MenuItem("Window/HDR Display Output")]
		public static void Open()
		{
			GetWindow<HDROutput>("HDR Display");
		}

		private void OnEnable()
		{
			InitializePlugin();
		}

		private void OnDisable()
		{
			FinalizePlugin();
		}

		private void OnGUI()
		{
			EditorGUI.BeginChangeCheck();

			EditorGUILayout.LabelField("HDR Display Output");
			if (GUILayout.Button("Show Window"))
			{
				if (!(_plugin?.IsAvailableDisplayWindow).GetValueOrDefault(false))
				{
					_requestCreateWindow = true;
				}
			}

			if (EditorGUI.EndChangeCheck())
			{
				//	property changed
			}
		}

		private void Update()
		{
			if (_requestCreateWindow)
			{
				_plugin?.CreateDisplayWindow(null);
				_requestCreateWindow = false;
				Repaint();
			}
		}

		private IEnumerable CreateDisplayWindow()
		{
			yield return new WaitForEndOfFrame();
		}

		private void InitializePlugin()
		{
			FinalizePlugin();
#if DYNAMIC_DLL_LOAD
			_dllManager = new DllManager("Assets/HDROutput/Plugins/x86_64/HDROutputPlugin.dll");
			_plugin = new HDROutputPlugin(_dllManager.GetDelegate<FnCreateHDROutputPluginInstance>("CreateHDROutputPluginInstance"));
#endif
		}

		private void FinalizePlugin()
		{
			_plugin?.Dispose();
			_plugin = null;

#if DYNAMIC_DLL_LOAD
			_dllManager?.Dispose();
			_dllManager = null;
#endif
		}

		private void SetCallbacks(bool regist)
		{
			if (regist)
			{
				_plugin?.SetCallbacks(
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
					},
					null);
			}
			else
			{
				_plugin?.SetCallbacks(null, null);
			}
		}
	}
}