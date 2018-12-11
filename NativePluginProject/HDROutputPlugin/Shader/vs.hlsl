#include "common.hlsli"

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT ret;

	ret.Pos = float4(input.Pos, 0.0, 1.0);
	ret.uv = input.uv;

	return ret;
}
