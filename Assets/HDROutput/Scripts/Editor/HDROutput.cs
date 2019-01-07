//#define DYNAMIC_DLL_LOAD

using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Threading;
using UnityEngine;
using UnityEditor;

namespace HDROutput
{
	public class HDROutput : EditorWindow
	{
#if DYNAMIC_DLL_LOAD
		private DllManager _dllManager = null;
#else
		[DllImport("HDROutputPlugin")]
		private static extern int CreateHDROutputPluginInstance(System.IntPtr[] buffer, int bufferSize);

		[DllImport("HDROutputPlugin")]
		private static extern System.IntPtr GetUnityRenderingEvent();
#endif

		private HDROutputPlugin _plugin = null;
		private Thread _thread = null;
		private SynchronizationContext _synccontext = null;
		private bool _isActiveThread = false;

		[SerializeField]
		private Texture _texture = null;

		[SerializeField]
		private bool _requestHDR = false;

		[SerializeField]
		private bool _convertColorSpace = false;

		[SerializeField]
		private bool _topmost = false;

		[SerializeField]
		private UnityEngine.RectInt? _previewWindowRect = null;

		[MenuItem("Window/HDR Display Output", false, 10000)]
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
			if (_thread != null && !_thread.Join(1))
			{
				_plugin.CloseWindow();
				_thread.Join();
			}
			FinalizePlugin();
		}

		private void OnGUI()
		{
			if (_plugin == null)
			{
				return;
			}

			EditorGUI.BeginChangeCheck();

			{
				var style = new GUIStyle();
				style.fontSize = 24;
				EditorGUI.LabelField(new Rect(0, 0, position.width, 24), "HDR Display Output", style);
			}

			if (GUI.Button(new Rect(position.width / 2 + 4, 36, position.width / 2 - 8, 56), "Open Window"))
			{
				if (_thread == null || _thread.Join(1))
				{
					_synccontext = SynchronizationContext.Current;
					_thread = new Thread(
						() =>
						{
							_isActiveThread = true;
							try
							{
								_previewWindowRect = _plugin.RunWindowProc(_previewWindowRect, OnDebugLog, OnPluginStateChanged);
							}
							finally
							{
								_isActiveThread = false;
								_synccontext = null;
							}
						});
					_thread.Start();
				}
			}

			_texture = EditorGUI.ObjectField(new Rect(0, 32, position.width / 2, 64), "Source Texture", _texture, typeof(Texture), true) as Texture;

			_convertColorSpace = EditorGUI.Toggle(new Rect(0, 96, position.width, 24), "Convert Color Space", _convertColorSpace);
			_requestHDR = EditorGUI.Toggle(new Rect(0, 120, position.width, 24), "Request HDR Output", _requestHDR);
			_topmost = EditorGUI.Toggle(new Rect(0, 144, position.width, 24), "Topmost", _topmost);

			if (_isActiveThread)
			{
				var isHDR = _plugin.IsAvailableHDR;
				var gammaCorrect = _convertColorSpace ? isHDR ?
					"BT.709 / Linear -> BT.2100 / PQ" :
					"BT.709 / Linear -> BT.709 / sRGB" :
					"None (Pass through)";

				EditorGUI.LabelField(
					new Rect(0, 168, position.width, 24),
					string.Format("Output:{0}, Convert Color Space:{1}", isHDR ? "HDR" : "SDR", gammaCorrect));
			}

			if (EditorGUI.EndChangeCheck())
			{
				//	property changed
				_plugin.SetSourceTexture(_texture != null ? _texture.GetNativeTexturePtr() : System.IntPtr.Zero);
				_plugin.ConvertColorSpace = _convertColorSpace;
				_plugin.RequestHDR = _requestHDR;
				_plugin.Topmost = _topmost;
			}
		}

		private void Update()
		{
			if (_texture != null)
			{
				_plugin?.UpdateSourceTextureAsync();
			}
		}

		private void OnDebugLog(PluginLogType logtype, string msg)
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
		}

		private void OnPluginStateChanged(PluginStateChanged state)
		{
			_synccontext?.Post(_ =>
			{
				switch (state)
				{
					case PluginStateChanged.CurrentHDRStateChanged:
						Repaint();
						break;
				}
			}, null);
		}

		private void InitializePlugin()
		{
			FinalizePlugin();
#if DYNAMIC_DLL_LOAD
			_dllManager = new DllManager("Assets/HDROutput/Plugins/x86_64/HDROutputPlugin.dll");
			_plugin = new HDROutputPlugin(
				_dllManager.GetDelegate<FnCreateHDROutputPluginInstance>("CreateHDROutputPluginInstance"),
				DllManager.GetProxyUnityRenderingEvent());
#else
			_plugin = new HDROutputPlugin(CreateHDROutputPluginInstance, GetUnityRenderingEvent());
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

	}
}