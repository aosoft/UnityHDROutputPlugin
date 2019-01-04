using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CubeBehaviour : MonoBehaviour
{
	// Use this for initialization
	void Start()
	{
	}

	// Update is called once per frame
	void Update()
	{
		gameObject.transform.Rotate(new Vector3(0.0f, Time.deltaTime * 10.0f, 0.0f));
	}
}
