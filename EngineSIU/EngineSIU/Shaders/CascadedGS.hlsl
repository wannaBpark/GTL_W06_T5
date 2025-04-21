#define MAX_CASCADE_NUM 3 // TO DO : TO FIX!!!!

cbuffer ConstantBufferShadowMap : register(b0)
{
    row_major matrix ShadowViewProj[MAX_CASCADE_NUM];
};

    struct GS_INPUT
{
    float4 pos : SV_POSITION;
};

struct PS_OUT_TEX_ARRAY
{
    float4 Position : SV_POSITION;
    uint RTIndex : SV_RenderTargetArrayIndex;
};

[maxvertexcount(3 * MAX_CASCADE_NUM)]
void mainGS( 
    triangle GS_INPUT input[3] : SV_POSITION, 
	inout TriangleStream<PS_OUT_TEX_ARRAY> TriStream
)
{
    PS_OUT_TEX_ARRAY output[3];
    for (uint i = 0; i < MAX_CASCADE_NUM; ++i)
    {
        for (uint j = 0; j < 3; j++)
        {
            float4 Position = mul(input[j].pos, ShadowViewProj[i]);
            Position.z += 2.5f; // bias value
            output[j].Position = Position;
            output[j].RTIndex = i;
            TriStream.Append(output[j]);
        }
        TriStream.RestartStrip();
    }
        
}
