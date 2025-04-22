
#define MAX_LIGHTS 16 

#define MAX_DIRECTIONAL_LIGHT 16
#define MAX_POINT_LIGHT 16
#define MAX_SPOT_LIGHT 16
#define MAX_AMBIENT_LIGHT 16

#define POINT_LIGHT         1
#define SPOT_LIGHT          2
#define DIRECTIONAL_LIGHT   3
#define AMBIENT_LIGHT       4

#define MAX_LIGHT_PER_TILE 1024

struct FAmbientLightInfo
{
    float4 AmbientColor;
};

struct FDirectionalLightInfo
{
    float4 LightColor;

    float3 Direction;
    float Intensity;
};

struct FPointLightInfo
{
    float4 LightColor;

    float3 Position;
    float Radius;

    int Type;
    float Intensity;
    float Attenuation;
    float Padding;
};

struct FSpotLightInfo
{
    float4 LightColor;

    float3 Position;
    float Radius;

    float3 Direction;
    float Intensity;

    int Type;
    float InnerRad;
    float OuterRad;
    float Attenuation;
    
    // --- Shadow Info ---
    row_major matrix LightViewProj; // 섀도우맵 생성 시 사용한 VP 행렬
    
    bool CastShadows;
    float ShadowBias;
    uint ShadowMapArrayIndex; // 필요시
    float Padding2; // 필요시
};

cbuffer FLightInfoBuffer : register(b0)
{
    FAmbientLightInfo Ambient[MAX_AMBIENT_LIGHT];
    FDirectionalLightInfo Directional[MAX_DIRECTIONAL_LIGHT];
    FPointLightInfo PointLights[MAX_POINT_LIGHT];
    FSpotLightInfo SpotLights[MAX_SPOT_LIGHT];
    
    int DirectionalLightsCount;
    int PointLightsCount;
    int SpotLightsCount;
    int AmbientLightsCount;
};

cbuffer TileLightCullSettings : register(b8)
{
    uint2 ScreenSize; // 화면 해상도
    uint2 TileSize; // 한 타일의 크기 (예: 16x16)

    float NearZ; // 카메라 near plane
    float FarZ; // 카메라 far plane

    row_major matrix TileViewMatrix; // View 행렬
    row_major matrix TileProjectionMatrix; // Projection 행렬
    row_major matrix TileInverseProjection; // Projection^-1, 뷰스페이스 복원용

    uint NumLights; // 총 라이트 수
    uint Enable25DCulling; // 1이면 2.5D 컬링 사용
}

struct LightPerTiles
{
    uint NumLights;
    uint Indices[MAX_LIGHT_PER_TILE];
    uint Padding[3];
};

StructuredBuffer<FPointLightInfo>   gPointLights    : register(t10);
StructuredBuffer<FSpotLightInfo>    gSpotLights     : register(t11);

Buffer<uint>    PerTilePointLightIndexBuffer    : register(t12);
Buffer<uint>    PerTileSpotLightIndexBuffer     : register(t13);



SamplerComparisonState ShadowSamplerCmp : register(s10);
Texture2DArray SpotShadowMapArray : register(t50);
Texture2DArray DirectionShadowMapArray : register(t51);

bool InRange(float val, float min, float max)
{
    return (min <= val && val <= max);
}

// float GetLightFromShadowMap(PS_INPUT_StaticMesh input)
// {
//     float NdotL = dot(normalize(input.WorldNormal), DirectionalLightDirection);
//     float bias = 0.001f * (1 - NdotL) + 0.0001f;
//
//     // float bias = 0.001f;
//     
//     float4 LightClipSpacePos = mul(float4(input.WorldPosition, 1.0f), ShadowViewProj);
//     float2 ShadowMapTexCoord = {
//         0.5f + LightClipSpacePos.x / LightClipSpacePos.w / 2.f,
//         0.5f - LightClipSpacePos.y / LightClipSpacePos.w / 2.f
//     };
//     float LightDistance = LightClipSpacePos.z / LightClipSpacePos.w;
//     LightDistance -= bias;
//
//     float Light = 0.f;
//     float OffsetX = 1.f / ShadowMapWidth;
//     float OffsetY = 1.f / ShadowMapHeight;
//     for(int i = -1; i <= 1; i++){
//         for(int j = -1; j <= 1; j++){
//             float2 SampleCoord =
//             {
//                 ShadowMapTexCoord.x + OffsetX * i,
//                 ShadowMapTexCoord.y + OffsetY * j
//             };
//             if (InRange(SampleCoord.x, 0.f, 1.f) && InRange(SampleCoord.y, 0.f, 1.f))
//             {
//                 Light += DirectionShadowMapArray.SampleCmpLevelZero(ShadowSamplerCmp, float3(SampleCoord, 0), LightDistance).r;
//             }
//             else
//             {
//                 Light += 1.f;
//             }
//         }
//     }
//     Light /= 9;
//     return Light;
//
//     // return ShadowMap.SampleCmpLevelZero(ShadowSampler, ShadowMapTexCoord, LightDistance).r;
// }
float CalculateDirectionalShadowFactor(float3 WorldPosition, float3 WorldNormal, FDirectionalLightInfo LightInfo, // 라이트 정보 전체 전달
                                Texture2DArray ShadowMapArray,
                                SamplerComparisonState ShadowSampler)
{
    float NdotL = dot(normalize(WorldNormal), LightInfo.Direction);
    float bias = 0.001f * (1 - NdotL) + 0.0001f;

    // float bias = 0.001f;
    
    float4 LightClipSpacePos = mul(float4(WorldPosition, 1.0f), ShadowViewProj);
    float2 ShadowMapTexCoord = {
        0.5f + LightClipSpacePos.x / LightClipSpacePos.w / 2.f,
        0.5f - LightClipSpacePos.y / LightClipSpacePos.w / 2.f
    };
    float LightDistance = LightClipSpacePos.z / LightClipSpacePos.w;
    LightDistance -= bias;

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
                // TODO: Cascade용으로 배열 인덱스 넣기
                Light += DirectionShadowMapArray.SampleCmpLevelZero(ShadowSamplerCmp, float3(SampleCoord, 0), LightDistance).r;
            }
            else
            {
                Light += 1.f;
            }
        }
    }
    Light /= 9;
    return Light;
}


// 기본적인 그림자 계산 함수 (Directional/Spot 용)
// 하드웨어 PCF (SamplerComparisonState 사용) 예시
float CalculateSpotShadowFactor(float3 WorldPosition, FSpotLightInfo LightInfo, // 라이트 정보 전체 전달
                                Texture2DArray ShadowMapArray,
                                SamplerComparisonState ShadowSampler)
{
    if (!LightInfo.CastShadows)
    {
        return 1.0f; // 그림자 안 드리움
    }

    // 1 & 2. 라이트 클립 공간 좌표 계산
    float4 PixelPosLightClip = mul(float4(WorldPosition, 1.0f), LightInfo.LightViewProj);

    // 3. 섀도우 맵 UV 계산 [0, 1]
    float2 ShadowMapUV = PixelPosLightClip.xy / PixelPosLightClip.w;
    ShadowMapUV = ShadowMapUV * float2(0.5, -0.5) + 0.5; // Y 반전 필요시

    // 4. 현재 깊이 계산
    float CurrentDepth = PixelPosLightClip.z / PixelPosLightClip.w;

    // UV 범위 체크 (라이트 범위 밖)
    if (any(ShadowMapUV < 0.0f) || any(ShadowMapUV > 1.0f))
    {
        return 1.0f;
    }

    // 5 & 6. 배열의 특정 슬라이스 샘플링 및 비교
    float ShadowFactor = ShadowMapArray.SampleCmpLevelZero(
        ShadowSampler,
        float3(ShadowMapUV, (float)LightInfo.ShadowMapArrayIndex), // UV와 배열 인덱스 사용
        CurrentDepth - LightInfo.ShadowBias // 바이어스 적용
    );

    return ShadowFactor;
}

float CalculateAttenuation(float Distance, float AttenuationFactor, float Radius)
{
    if (Distance > Radius)
    {
        return 0.0;
    }

    float Falloff = 1.0 / (1.0 + AttenuationFactor * Distance * Distance);
    float SmoothFactor = (1.0 - (Distance / Radius)); // 부드러운 falloff

    return Falloff * SmoothFactor;
}

float CalculateSpotEffect(float3 LightDir, float3 SpotDir, float InnerRadius, float OuterRadius, float SpotFalloff)
{
    float Dot = dot(-LightDir, SpotDir); // [-1,1]
    
    float SpotEffect = smoothstep(cos(OuterRadius / 2), cos(InnerRadius / 2), Dot);
    
    return SpotEffect * pow(max(Dot, 0.0), SpotFalloff);
}

float CalculateDiffuse(float3 WorldNormal, float3 LightDir)
{
    return max(dot(WorldNormal, LightDir), 0.0);
}

float CalculateSpecular(float3 WorldNormal, float3 ToLightDir, float3 ViewDir, float Shininess, float SpecularStrength = 0.5)
{
#ifdef LIGHTING_MODEL_GOURAUD
    float3 ReflectDir = reflect(-ToLightDir, WorldNormal);
    float Spec = pow(max(dot(ViewDir, ReflectDir), 0.0), Shininess);
#else
    float3 HalfDir = normalize(ToLightDir + ViewDir); // Blinn-Phong
    float Spec = pow(max(dot(WorldNormal, HalfDir), 0.0), Shininess);
#endif
    return Spec * SpecularStrength; 
}

float4 PointLight(int Index, float3 WorldPosition, float3 WorldNormal, float WorldViewPosition, float3 DiffuseColor)
{
    FPointLightInfo LightInfo = gPointLights[Index];
    //FPointLightInfo LightInfo = PointLights[Index];
    
    float3 ToLight = LightInfo.Position - WorldPosition;
    float Distance = length(ToLight);
    
    float Attenuation = CalculateAttenuation(Distance, LightInfo.Attenuation, LightInfo.Radius);
    if (Attenuation <= 0.0)
    {
        return float4(0.f, 0.f, 0.f, 0.f);
    }
    
    float3 LightDir = normalize(ToLight);
    float DiffuseFactor = CalculateDiffuse(WorldNormal, LightDir);
#ifdef LIGHTING_MODEL_LAMBERT
    float3 Lit = (DiffuseFactor * DiffuseColor) * LightInfo.LightColor.rgb;
#else
    float3 ViewDir = normalize(WorldViewPosition - WorldPosition);
    float SpecularFactor = CalculateSpecular(WorldNormal, LightDir, ViewDir, Material.SpecularScalar);
    float3 Lit = ((DiffuseFactor * DiffuseColor) + (SpecularFactor * Material.SpecularColor)) * LightInfo.LightColor.rgb;
#endif
    
    return float4(Lit * Attenuation * LightInfo.Intensity, 1.0);
}

float4 SpotLight(int Index, float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition, float3 DiffuseColor)
{
    FSpotLightInfo LightInfo = gSpotLights[Index];
    // FSpotLightInfo LightInfo = SpotLights[Index];
    
    float3 ToLight = LightInfo.Position - WorldPosition;
    float Distance = length(ToLight);
    
    float Attenuation = CalculateAttenuation(Distance, LightInfo.Attenuation, LightInfo.Radius);
    if (Attenuation <= 0.0)
    {
        return float4(0.0, 0.0, 0.0, 0.0);
    }
    
    float3 LightDir = normalize(ToLight);
    float SpotlightFactor = CalculateSpotEffect(LightDir, normalize(LightInfo.Direction), LightInfo.InnerRad, LightInfo.OuterRad, LightInfo.Attenuation);
    if (SpotlightFactor <= 0.0)
    {
        return float4(0.0, 0.0, 0.0, 0.0);
    }

    // --- 그림자 계산 
    float Shadow = CalculateSpotShadowFactor(WorldPosition, LightInfo, SpotShadowMapArray, ShadowSamplerCmp);

    // 그림자 계수가 0 이하면 더 이상 계산 불필요
    if (Shadow <= 0.0)
    {
        return float4(0.0, 0.0, 0.0, 0.0);
    }
    
    float DiffuseFactor = CalculateDiffuse(WorldNormal, LightDir);
    
#ifdef LIGHTING_MODEL_LAMBERT
    float3 Lit = DiffuseFactor * DiffuseColor * LightInfo.LightColor.rgb;
#else
    float3 ViewDir = normalize(WorldViewPosition - WorldPosition);
    float SpecularFactor = CalculateSpecular(WorldNormal, LightDir, ViewDir, Material.SpecularScalar);
    float3 Lit = ((DiffuseFactor * DiffuseColor) + (SpecularFactor * Material.SpecularColor)) * LightInfo.LightColor.rgb;
#endif
    
    return float4(Lit * Attenuation * SpotlightFactor * LightInfo.Intensity * Shadow, 1.0);
}

float4 DirectionalLight(int nIndex, float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition, float3 DiffuseColor)
{
    FDirectionalLightInfo LightInfo = Directional[nIndex];
    
    float3 LightDir = normalize(-LightInfo.Direction);
    float3 ViewDir = normalize(WorldViewPosition - WorldPosition);
    float DiffuseFactor = CalculateDiffuse(WorldNormal, LightDir);

    
    //FinalColor *= GetLightFromShadowMap(Input);
    float Shadow =  CalculateDirectionalShadowFactor(WorldPosition, WorldNormal, LightInfo, DirectionShadowMapArray, ShadowSamplerCmp);
    
    // 그림자 계수가 0 이하면 더 이상 계산 불필요
    if (Shadow <= 0.0)
    {
        return float4(0.0, 0.0, 0.0, 0.0);
    }
    
#ifdef LIGHTING_MODEL_LAMBERT
    float3 Lit = DiffuseFactor * DiffuseColor * LightInfo.LightColor.rgb;
#else
    float SpecularFactor = CalculateSpecular(WorldNormal, LightDir, ViewDir, Material.SpecularScalar);
    float3 Lit = ((DiffuseFactor * DiffuseColor) + (SpecularFactor * Material.SpecularColor)) * LightInfo.LightColor.rgb;
#endif
    return float4(Lit * Shadow * LightInfo.Intensity, 1.0);
}

float4 Lighting(float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition, float3 DiffuseColor, uint TileIndex)
{
    float4 FinalColor = float4(0.0, 0.0, 0.0, 0.0);

    int BucketsPerTile = MAX_LIGHT_PER_TILE / 32;
    int StartIndex = TileIndex * BucketsPerTile;
    for (int Bucket = 0; Bucket < BucketsPerTile; ++Bucket)
    {
        int PointMask = PerTilePointLightIndexBuffer[StartIndex + Bucket];
        int SpotMask = PerTileSpotLightIndexBuffer[StartIndex + Bucket];
        for (int bit = 0; bit < 32; ++bit)
        {
            if (PointMask & (1u << bit))
            {
                // 전역 조명 인덱스는 bucket * 32 + bit 로 계산됨.
                // 전역 조명 인덱스가 총 조명 수보다 작은 경우에만 추가
                int GlobalPointLightIndex = Bucket * 32 + bit;
                if (GlobalPointLightIndex < MAX_LIGHT_PER_TILE)
                {
                    FinalColor += PointLight(GlobalPointLightIndex, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor);
                }
            }
            if (SpotMask & (1u << bit))
            {
                int GlobalSpotLightIndex = Bucket * 32 + bit;
                if (GlobalSpotLightIndex < MAX_LIGHT_PER_TILE)
                {
                    FinalColor += SpotLight(GlobalSpotLightIndex, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor);
                }
            }
        }
    }
    
    // [unroll(MAX_SPOT_LIGHT)]
    // for (int j = 0; j < SpotLightsCount; j++)
    // {
    //     FinalColor += SpotLight(j, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor);
    // }
    [unroll(MAX_DIRECTIONAL_LIGHT)]
    for (int k = 0; k < 1; k++)
    {
        //FinalColor += float4(1.0f,1.0f,1.0f,1.0f);
        FinalColor += DirectionalLight(k, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor);
    }
    [unroll(MAX_AMBIENT_LIGHT)]
    for (int l = 0; l < AmbientLightsCount; l++) 
    {
        FinalColor += float4(Ambient[l].AmbientColor.rgb * DiffuseColor, 0.0);
        FinalColor.a = 1.0;
    }
    
    return FinalColor;
}

float4 Lighting(float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition, float3 DiffuseColor)
{
    float4 FinalColor = float4(0.0, 0.0, 0.0, 0.0);

    // 다소 비효율적일 수도 있음.
    [unroll(MAX_POINT_LIGHT)]
    for (int i = 0; i < PointLightsCount; i++)
    {
        FinalColor += PointLight(i, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor);
    }

    [unroll(MAX_SPOT_LIGHT)]
    for (int j = 0; j < SpotLightsCount; j++)
    {
        FinalColor += SpotLight(j, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor);
    }
    [unroll(MAX_DIRECTIONAL_LIGHT)]
    for (int k = 0; k < 1; k++)
    {
        FinalColor += DirectionalLight(k, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor);
    }
    [unroll(MAX_AMBIENT_LIGHT)]
    for (int l = 0; l < AmbientLightsCount; l++)
    {
        FinalColor += float4(Ambient[l].AmbientColor.rgb * DiffuseColor, 0.0);
        FinalColor.a = 1.0;
    }
    
    return FinalColor;
}

// 그림자 계산

/* 
float CalShadowFactor(float3 WorldPos, float ShadowBias)
{
    const float nearZ = 0.01; // 카메라 설정과 동일
    float shadowFactor;
    // 1. Project posWorld to light screen    
    float4 LightScreen = mul(float4(WorldPos, 1.0), ShadowViewProj);
    LightScreen.xyz /= LightScreen.w;
    
    // 2. 카메라(광원)에서 볼 때의 Texture 좌표 계산
    float2 LightTexcoord = float2(LightScreen.x, -LightScreen.y);
    LightTexcoord += 1.0;
    LightTexcoord *= 0.5;
    
    // 3. 쉐도우맵에서 값 가져오기
    float mapDepth = shadowMap.Sample(shadowPointSampler, LightTexcoord).r;
    float actualDepth = LightScreen.z;
    
    // 4. ShadowMap의 depth 보다 실제 depth 가 크다면 그림자
    if (mapDepth + bias < actualDepth)
        shadowFactor = 0.0;
    
    
    shadowMap.SampleCmpLevelZero(shadowCompareSampler, LightTexcoord.xy, LightScreen.z - 0.001).r;

    uint width, height, numMips;
    shadowMap.GetDimensions(0, width, height, numMips);
    float dx = 5.0 / (float) width;
    float percentLit = 0.0;
    const float2 offsets[9] =
    {
        float2(-1, -1), float2(0, -1), float2(1, -1),
        float2(-1, 0), float2(0, 0), float2(1, 0),
        float2(-1, +1), float2(0, +1), float2(1, +1)
    };
    
    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += shadowMap.SampleCmpLevelZero(shadowCompareSampler,
            LightTexcoord.xy + offsets[i] * dx, LightScreen.z - 0.001
        ).r;

    }
    shadowFactor = percentLit / 9.0;

    return shadowFactor;
}*/
