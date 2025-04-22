// Depth Only Vertex Shader

#include "ShaderRegisters.hlsl"

float4 mainVS(VS_INPUT_StaticMesh Input) : SV_POSITION
{
    float4 pos = mul(float4(Input.Position, 1.0f), WorldMatrix);
    pos = mul(pos, ShadowViewProj);
	return pos;
}
