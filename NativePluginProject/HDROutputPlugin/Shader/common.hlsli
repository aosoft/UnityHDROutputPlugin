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
