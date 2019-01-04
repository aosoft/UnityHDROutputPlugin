#include "common.hlsli"
#include "color.hlsli"

float4 main(PS_INPUT input) : SV_Target
{
	const float standardNits = 80.0;
	const float st2084max = 10000.0;
	const float hdrScalar = standardNits / st2084max;

	float4 color = tex.Sample(texSampler, input.uv);
	return float4(LinearToST2084(Rec709ToRec2020(color.rgb) * hdrScalar), 1.0);
}
