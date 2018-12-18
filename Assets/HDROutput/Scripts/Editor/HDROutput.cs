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

		[SerializeField]
		private Texture _texture;

		[SerializeField]
		UnityEngine.RectInt? _previewWindowRect = null;

		[MenuItem("Window/HDR Display Output")]
		public static void Open()
		{
			GetWindow<HDROutput>("HDR Display");
		}

		private void OnEnable()
		{
			InitializePlugin();
			_plugin?.SetCallbacks(OnDebugLog, OnPluginStateChanged);
		}

		private void OnDisable()
		{
			FinalizePlugin();
		}

		private void OnGUI()
		{
			if (_plugin == null)
			{
				return;
			}

			EditorGUI.BeginChangeCheck();

			EditorGUILayout.LabelField("HDR Display Output");
			if (GUILayout.Button("Open Window"))
			{
				if (!_plugin.IsAvailableDisplayWindow)
				{
					_plugin.CreateDisplayWindow(_previewWindowRect);
				}
			}

			_texture = EditorGUILayout.ObjectField("Source Texture", _texture, typeof(Texture), true) as Texture;

			if (EditorGUI.EndChangeCheck())
			{
				//	property changed
				_plugin.SetSourceTexture(_texture != null ? _texture.GetNativeTexturePtr() : System.IntPtr.Zero);
			}

		}

		private void Update()
		{
			if (_texture != null)
			{
				_plugin?.RenderAsync();
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
			switch (state)
			{
				case PluginStateChanged.WindowSizeChanged:
					//_plugin?.RenderAsync();
					break;

				case PluginStateChanged.WindowClosing:
					if (_plugin != null)
					{
						_previewWindowRect = _plugin.GetWindowRect();
					}
					break;

				case PluginStateChanged.CurrentHDRStateChanged:
					Repaint();
					break;
			}
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

	}
}