#include "common.hlsli"
#include "color.hlsli"

float4 main(PS_INPUT input) : SV_Target
{
	float4 color = tex.Sample(texSampler, input.uv);
	return float4(Rec709ToRec2020(LinearToST2084(color.rgb)), 1.0);
}
