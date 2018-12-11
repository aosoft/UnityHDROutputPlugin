struct VS_INPUT
{
	float2 Pos: POSITION;
	float2 uv:  TEXCOORD0;
};

struct PS_INPUT
{
	float4 Pos: SV_POSITION;
	float2 uv:  TEXCOORD0;
};

Texture2D tex : register(t0);
SamplerState texSampler : register(s0);
