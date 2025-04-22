
#ifndef SHADER_REGISTER_INCLUDE
#define SHADER_REGISTER_INCLUDE

float LinearToSRGB(float val)
{
    float low  = 12.92 * val;
    float high = 1.055 * pow(val, 1.0 / 2.4) - 0.055;
    // linear가 임계값보다 큰지 판별 후 선형 보간
    float t = step(0.0031308, val); // linear >= 0.0031308이면 t = 1, 아니면 t = 0
    return lerp(low, high, t);
}

float3 LinearToSRGB(float3 color)
{
    color.r = LinearToSRGB(color.r);
    color.g = LinearToSRGB(color.g);
    color.b = LinearToSRGB(color.b);
    return color;
}

float SRGBToLinear(float val)
{
    float low  = val / 12.92;
    float high = pow((val + 0.055) / 1.055, 2.4);
    float t = step(0.04045, val); // srgb가 0.04045 이상이면 t = 1, 아니면 t = 0
    return lerp(low, high, t);
}

float3 SRGBToLinear(float3 color)
{
    color.r = SRGBToLinear(color.r);
    color.g = SRGBToLinear(color.g);
    color.b = SRGBToLinear(color.b);
    return color;
}

struct FMaterial
{
    float3 DiffuseColor;
    float TransparencyScalar;
    
    float3 SpecularColor;
    float SpecularScalar;
    
    float3 EmissiveColor;
    float DensityScalar;
    
    float3 AmbientColor;
    uint TextureFlag;
};

struct VS_INPUT_StaticMesh
{
    float3 Position : POSITION;
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 UV : TEXCOORD;
    uint MaterialIndex : MATERIAL_INDEX;
};

struct PS_INPUT_StaticMesh
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float3 WorldNormal : NORMAL;
    float2 UV : TEXCOORD0;
    float3 WorldPosition : TEXCOORD1;
    float3 WorldViewPosition : TEXCOORD2;
    float3x3 TBN : TBN;
    int MaterialIndex : MATERIAL_INDEX;
};

////////
/// 공용: 11 ~ 13
///////

cbuffer ShadowMapBuffer : register(b11)
{
    row_major matrix ShadowViewProj;
    row_major matrix ShadowInvProj;

    float ShadowMapWidth;
    float ShadowMapHeight;
    float2 ShadowPadding0;

    float3 DirectionalLightDir;
    float ShadowPadding1;

    float3 LightPos;
    float ShadowPadding2;

    float4 gCascadeSplits;

};

cbuffer ObjectBuffer : register(b12)
{
    row_major matrix WorldMatrix;
    row_major matrix InverseTransposedWorld;
    
    float4 UUID;
    
    bool bIsSelected;
    float3 ObjectPadding;
};

/**
 * 기존에는 View 버퍼와 Projection 버퍼의
 * 업데이트 시기가 달라서 분리했지만, 
 * 여러 뷰포트를 렌더하는 경우 다음 뷰포트로 넘어가면
 * Projection 버퍼를 업데이트해야 하므로,
 * 버퍼를 분리하는 의미가 사라졌으므로 통합.
 */
cbuffer CameraBuffer : register(b13)
{
    row_major matrix ViewMatrix;
    row_major matrix InvViewMatrix;
    
    row_major matrix ProjectionMatrix;
    row_major matrix InvProjectionMatrix;
    
    float3 ViewWorldLocation; // TODO: 가능하면 버퍼에서 빼기
    float ViewPadding;
    
    float NearClip;
    float FarClip;
    float2 ProjectionPadding;
}

#endif
