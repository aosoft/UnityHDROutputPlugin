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

		[DllImport("user32.dll")]
		private static extern System.IntPtr GetAncestor(System.IntPtr hwnd, int flags);

		[DllImport("user32.dll", SetLastError = true)]
		static extern System.IntPtr GetActiveWindow();

		private HDROutputPlugin _plugin = null;
		private Thread _thread = null;
		private SynchronizationContext _synccontext = null;
		private bool _isActiveThread = false;

		[SerializeField]
		private Texture _texture = null;

		[SerializeField]
		private PluginColorSpace _requestColorSpace = PluginColorSpace.sRGB;

		[SerializeField]
		private bool _convertColorSpace = false;

		[SerializeField]
		private float _relativeEV = 0.0f;

		[SerializeField]
		private UnityEngine.RectInt? _previewWindowRect = null;

		private static string[] _colorspaceNames = new string[]{
			"(SDR) sRGB",
			"(HDR) BT.2100 PQ",
			"(HDR) BT.709 Linear"
		};

		private static int[] _colorspaceValues = new int[]{
			(int)PluginColorSpace.sRGB,
			(int)PluginColorSpace.BT2100_PQ,
			(int)PluginColorSpace.BT709_Linear,
		};


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
					var parent = GetActiveWindow();
					if (parent != System.IntPtr.Zero)
					{
						parent = GetAncestor(parent, 3);
					}
					_thread = new Thread(
						() =>
						{
							_isActiveThread = true;
							try
							{
								_previewWindowRect = _plugin.RunWindowProc(parent, _previewWindowRect, OnDebugLog, OnPluginStateChanged);
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
			EditorGUI.LabelField(new Rect(0, 120, 150, 24), "Requese Color Space");
			_requestColorSpace = (PluginColorSpace)EditorGUI.IntPopup(
				new Rect(150, 120, position.width - 150, 24),
				(int)_requestColorSpace,
				_colorspaceNames,
				_colorspaceValues);

			float sliderWidth = Mathf.Max(position.width - 52, 0);
			_relativeEV = EditorGUI.Slider(new Rect(0, 144, sliderWidth, 18), "Relative EV", _relativeEV, -8.0f, 8.0f);
			if (GUI.Button(new Rect(sliderWidth + 4, 144, 44, 18), "Reset"))
			{
				_relativeEV = 0.0f;
			}

			if (_isActiveThread)
			{
				var activeColorSpace = _plugin.ActiveColorSpace;
				var gammaCorrect = "None (Pass through)";
				if (_convertColorSpace)
				{
					switch (activeColorSpace)
					{
						case PluginColorSpace.sRGB:
							gammaCorrect = "BT.709 / Linear -> BT.709 / sRGB (SDR)";
							break;
						case PluginColorSpace.BT2100_PQ:
							gammaCorrect = "BT.709 / Linear -> BT.2100 / PQ (HDR)";
							break;
						case PluginColorSpace.BT709_Linear:
							gammaCorrect = "BT.709 / Linear -> BT.709 / Linear (HDR)";
							break;

					}
				}

				EditorGUI.LabelField(
					new Rect(0, 168, position.width, 24),
					string.Format("Convert Color Space:{0}", gammaCorrect));
			}

			if (EditorGUI.EndChangeCheck())
			{
				//	property changed
				_plugin.SetSourceTexture(_texture != null ? _texture.GetNativeTexturePtr() : System.IntPtr.Zero);
				_plugin.ConvertColorSpace = _convertColorSpace;
				_plugin.RequestColorSpace = _requestColorSpace;
				_plugin.RelativeEV = _relativeEV;
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