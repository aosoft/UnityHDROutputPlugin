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

cbuffer cbuffer0 : register(b0)
{
	float relativeEV;
};

Texture2D tex : register(t0);
SamplerState texSampler : register(s0);


float4 GetPixel(float2 uv)
{
	float4 color = tex.Sample(texSampler, uv);
	return float4(color.rgb * pow(2, relativeEV), color.a);
}
