// depthonlyvs

struct VS_INPUT_StaticMesh
{
    float3 Position : POSITION;
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 UV : TEXCOORD;
    uint MaterialIndex : MATERIAL_INDEX;
};

float4 mainVS( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}
