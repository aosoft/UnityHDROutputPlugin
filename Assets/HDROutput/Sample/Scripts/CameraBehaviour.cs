using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CameraBehaviour : MonoBehaviour
{
	public RenderTexture _renderTexture;

	private Camera _camera;

	private void Awake()
	{
		_camera = GetComponent<Camera>();
	}

	// Update is called once per frame
	void Update()
	{
		if (_camera != null && _renderTexture != null)
		{
			_camera.targetTexture = _renderTexture;
			_camera.Render();
			_camera.targetTexture = null;
		}
	}
}
