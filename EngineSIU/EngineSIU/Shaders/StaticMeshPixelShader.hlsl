
#include "ShaderRegisters.hlsl"

SamplerState DiffuseSampler : register(s0);
SamplerState NormalSampler : register(s1);
SamplerComparisonState ShadowSampler : register(s2);
SamplerState ShadowPointSampler : register(s3);

Texture2D DiffuseTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D ShadowMap : register(t2);

#define NEAR_PLANE 1
#define LIGHT_RADIUS_WORLD 20000
cbuffer MaterialConstants : register(b1)
{
    FMaterial Material;
}

cbuffer FlagConstants : register(b2)
{
    bool IsLit;
    float3 flagPad0;
}

cbuffer SubMeshConstants : register(b3)
{
    bool IsSelectedSubMesh;
    float3 SubMeshPad0;
}

cbuffer TextureConstants : register(b4)
{
    float2 UVOffset;
    float2 TexturePad0;
}

#include "Light.hlsl"


bool InRange(float val, float min, float max)
{
    return (min <= val && val <= max);
}


float PCF_Filter(float2 uv, float zReceiverNdc, float filterRadiusUV, Texture2D shadowMap)
{
    float sum = 0.0f;
    [unroll]
    for (int i = 0; i < 64; ++i)
    {
        float2 offset = diskSamples64[i] * filterRadiusUV;
        sum += shadowMap.SampleCmpLevelZero(
            ShadowSampler, uv + offset, zReceiverNdc);
    }
    return sum / 64;
}

void FindBlocker(out float avgBlockerDepthView, out float numBlockers, float2 uv,
                 float zReceiverView, Texture2D shadowMap, matrix InvProj, float LightRadiusWorld)
{
    float LightRadiusUV = LightRadiusWorld / LightFrustumWidth; // TO FIX!
    float searchRadius = LightRadiusUV * (zReceiverView - NEAR_PLANE) / zReceiverView; // TO FIX! NearPlane
    float blockerSum = 0;
    numBlockers = 0;

    for (int i = 0; i < 64; ++i)
    {
        float ShadowMapDepth = ShadowMap.SampleLevel(ShadowPointSampler, float2(uv + diskSamples64[i] * searchRadius), 0).r;
        ShadowMapDepth = N2V(ShadowMapDepth, InvProj);
        if (ShadowMapDepth < zReceiverView)
        {
            blockerSum += ShadowMapDepth;
            numBlockers += 1;
        }
    }
    avgBlockerDepthView = (numBlockers > 0) ? (blockerSum / numBlockers) : 0.0f;
}

float PCSS(float2 uv, float zReceiverNDC, Texture2D ShadowMap, matrix ShadowInvProj, float LightRadiusWorld)
{
    float lightRadiusUV = LightRadiusWorld / LightFrustumWidth;       // TO FIX!
    float zReceiverView = N2V(zReceiverNDC, ShadowInvProj);

    // 1. Blocker Search
    float avgBlockerDepthView = 0;
    float numBlockers = 0;
    FindBlocker(avgBlockerDepthView, numBlockers, uv, zReceiverView, ShadowMap, ShadowInvProj, LightRadiusWorld);

    if (numBlockers<1)
    {
        // There are no Occluders so early out(this saves filtering)
        return 1.0f;
    }
    else 
    {
        // 2. Penumbra Size
        float penumbraRatio = (zReceiverView - avgBlockerDepthView) / avgBlockerDepthView;
        float filterRadiusUV = penumbraRatio * lightRadiusUV * NEAR_PLANE / zReceiverView; // TO FIX!!!!

        // 3. Filtering
        return PCF_Filter(uv, zReceiverNDC, filterRadiusUV, ShadowMap);
    }
}


float GetLightFromShadowMap(PS_INPUT_StaticMesh input)
{
    //return 0.0f;
    // float NdotL = dot(normalize(input.WorldNormal), DirectionalLightDir);
    // float bias = 0.001f * (1 - NdotL) + 0.0001f;
    float ShadowFactor = 1.0;
    float bias = 0.001f;

    // 1. Project World Position to Light Screen Space
    float4 LightScreen = mul(float4(input.WorldPosition, 1.0f), ShadowViewProj);
    LightScreen.xyz /= LightScreen.w; // Perspective Divide -> [-1, 1] 범위로 변환

    // 2. 광원 입장의 Texture 좌표계로 변환
    float2 ShadowMapTexCoord = { LightScreen.x, -LightScreen.y }; // NDC 좌표계와 UV 좌표계는 Y축 방향이 반대
    ShadowMapTexCoord += 1.0;
    ShadowMapTexCoord *= 0.5;

    float LightDistance = LightScreen.z;
    LightDistance -= bias;

    uint width, height, numMips;
    ShadowMap.GetDimensions(0, width, height, numMips);
    // Texel Size 계산
    float dx = 5.0 / (float) width;

    ShadowFactor = PCSS(ShadowMapTexCoord, LightDistance, ShadowMap, ShadowInvProj, LIGHT_RADIUS_WORLD);
    return ShadowFactor;

    float Light = 0.f;
    float OffsetX = 1.f / ShadowMapWidth;
    float OffsetY = 1.f / ShadowMapHeight;
    for(int i = -1; i <= 1; i++){
        for(int j = -1; j <= 1; j++){
            float2 SampleCoord =
            {
                ShadowMapTexCoord.x + OffsetX * i,
                ShadowMapTexCoord.y + OffsetY * j
            };
            if (InRange(SampleCoord.x, 0.f, 1.f) && InRange(SampleCoord.y, 0.f, 1.f))
            {
                Light += ShadowMap.SampleCmpLevelZero(ShadowSampler, SampleCoord, LightDistance).r;
            }
            else
            {
                Light += 1.f;
            }
        }
    }
    Light /= 9;
    return Light;

    return ShadowMap.SampleCmpLevelZero(ShadowSampler, ShadowMapTexCoord, LightDistance).r;
}

float4 mainPS(PS_INPUT_StaticMesh Input) : SV_Target
{
    float4 FinalColor = float4(0.f, 0.f, 0.f, 1.f);

    // Diffuse
    float3 DiffuseColor = Material.DiffuseColor;
    if (Material.TextureFlag & (1 << 1))
    {
        DiffuseColor = DiffuseTexture.Sample(DiffuseSampler, Input.UV).rgb;
        DiffuseColor = SRGBToLinear(DiffuseColor);
    }

    // Normal
    float3 WorldNormal = Input.WorldNormal;
    if (Material.TextureFlag & (1 << 2))
    {
        float3 Normal = NormalTexture.Sample(NormalSampler, Input.UV).rgb;
        Normal = normalize(2.f * Normal - 1.f);
        WorldNormal = mul(mul(Normal, Input.TBN), (float3x3) InverseTransposedWorld);
    }
    WorldNormal = normalize(WorldNormal);

    // Begin for Tile based light culled result
    // 현재 픽셀이 속한 타일 계산 (input.position = 화면 픽셀좌표계)
    uint2 PixelCoord = uint2(Input.Position.xy);
    uint2 TileCoord = PixelCoord / TileSize; // 각 성분별 나눔
    uint TilesX = ScreenSize.x / TileSize.x; // 한 행에 존재하는 타일 수
    uint FlatTileIndex = TileCoord.x + TileCoord.y * TilesX;
    // End for Tile based light culled result

    // Lighting
    if (IsLit)
    {
#ifdef LIGHTING_MODEL_GOURAUD
        FinalColor = float4(Input.Color.rgb * DiffuseColor, 1.0);
#else
        float3 LitColor = Lighting(Input.WorldPosition, WorldNormal, Input.WorldViewPosition, DiffuseColor, FlatTileIndex).rgb;
        FinalColor = float4(LitColor, 1);
#endif
    }
    else
    {
        FinalColor = float4(DiffuseColor, 1);
    }
    
    if (bIsSelected)
    {
        FinalColor += float4(0.01, 0.01, 0.0, 1);
    }

    // Shadow
    //FinalColor *= GetLightFromShadowMap(Input);

    return FinalColor;
}
