#include "ComputeDefine.hlsl"

StructuredBuffer<FPointLightGPU> PointLightBuffer : register(t0); // CPU에서 전달된 PointLight들
StructuredBuffer<FSpotLightGPU> SpotLightBuffer : register(t2); // CPU에서 전달된 SpotLight들
Texture2D<float> gDepthTexture : register(t1); // Depth Texture

RWStructuredBuffer<uint> TileLightMask : register(u0); // 타일별 Point Light 마스크
RWTexture2D<float4> DebugHeatmap : register(u3); // 디버깅용 히트맵
RWStructuredBuffer<uint> TileSpotLightMask : register(u6); // 타일별 Spot Light 마스크

// Group Shared 메모리 - Depth Masking 누적에 쓰입니다
groupshared uint tileDepthMask;

// Group(타일) 단위로 오브젝트의 minZ, maxZ 값 저장
groupshared uint groupMinZ; // float max (≈ 3.4e+38);
groupshared uint groupMaxZ; // float min (0);
groupshared uint hitCount;

// PointLight, SpotLight 컬링 함수
bool ShouldLightAffectTile(float3 lightVSPos, float radius, float minZ, float maxZ, uint depthMask)
{
    if (depthMask == 0)
        return false;

    float s_minDepth = lightVSPos.z - radius;
    float s_maxDepth = lightVSPos.z + radius;

    if (s_maxDepth < minZ || s_minDepth > maxZ)
        return false;

    float normMin = saturate((s_minDepth - minZ) / max(1e-5, maxZ - minZ));
    float normMax = saturate((s_maxDepth - minZ) / max(1e-5, maxZ - minZ));

    int sliceMin = (int) floor(normMin * NUM_SLICES);
    int sliceMax = (int) ceil(normMax * NUM_SLICES);
    sliceMin = clamp(sliceMin, 0, NUM_SLICES - 1);
    sliceMax = clamp(sliceMax, 0, NUM_SLICES - 1);

    uint sphereMask = 0;
    [unroll]
    for (int i = sliceMin; i <= sliceMax; ++i)
        sphereMask |= (1u << i);

    return (sphereMask & depthMask) != 0;
}

// 각 조명에 대해 컬링 및 마스킹 적용
void CullLight(uint index, float3 lightVSPos, float radius, Frustum frustum, float minZ, float maxZ, uint flatTileIndex, RWStructuredBuffer<uint> MaskBuffer)
{
    Sphere s = { lightVSPos, radius };

    if (!SphereInsideFrustum(s, frustum, NearZ, FarZ))
        return;
    if (Enable25DCulling != 0 && !ShouldLightAffectTile(lightVSPos, radius, minZ, maxZ, tileDepthMask))
        return;

    uint bucketIdx = index / 32;
    uint bitIdx = index % 32;
    InterlockedOr(MaskBuffer[flatTileIndex * SHADER_ENTITY_TILE_BUCKET_COUNT + bucketIdx], 1 << bitIdx);
    InterlockedAdd(hitCount, 1);
}

// 타일에 존재하는 Point 및 Spot 라이트에 대해 컬링
void CullLightsForTile(Frustum frustum, float minZ, float maxZ, uint flatTileIndex)
{
    [loop]
    for (uint i = 0; i < NumPointLights; ++i)
    {
        float3 lightVSPos = mul(float4(PointLightBuffer[i].Position, 1), View).xyz;
        CullLight(i, lightVSPos, PointLightBuffer[i].Radius, frustum, minZ, maxZ, flatTileIndex, TileLightMask);
    }

    [loop]
    for (uint j = 0; j < NumSpotLights; ++j)
    {
        float3 lightVSPos = mul(float4(SpotLightBuffer[j].Position, 1), View).xyz;
        CullLight(j, lightVSPos, SpotLightBuffer[j].Radius, frustum, minZ, maxZ, flatTileIndex, TileSpotLightMask);
    }
}

// 히트맵 결과 색상 렌더링
void WriteHeatmap(uint2 tileCoord)
{
    float4 result = float4(0, 0, 0, 0); // 기본: 아무것도 안 보임

    // 히트된 라이트가 있는 경우만 시각화 
    if (hitCount > 0) {
        const float3 heatmap[] = {
            float3(0, 0, 0), float3(0, 0, 1), float3(0, 1, 1),
            float3(0, 1, 0), float3(1, 1, 0), float3(1, 0, 0)
        };
        const float maxHeat = 50.0f;
        float l = saturate(hitCount / maxHeat) * 5;
        float3 c1 = heatmap[floor(l)];
        float3 c2 = heatmap[ceil(l)];
        result = float4(lerp(c1, c2, frac(l)), 0.8f);
    }

    // 타일 전체에 결과 색상 출력
    for (uint i = 0; i < TILE_SIZE * TILE_SIZE; ++i) {
        uint2 local = unflatten2D(i, uint2(TILE_SIZE, TILE_SIZE));
        uint2 pixel = tileCoord * TILE_SIZE + local;
        if (all(pixel < ScreenSize)) {
            DebugHeatmap[pixel] = result;
        }
    }
}

// CullLightsForTile을 호출하여 라이트 컬링을 수행
[numthreads(TILE_SIZE, TILE_SIZE, 1)]
void mainCS(uint3 groupID : SV_GroupID, uint3 dispatchID : SV_DispatchThreadID, uint3 threadID : SV_GroupThreadID)
{
    uint2 tileCoord = groupID.xy;
    uint2 pixel = tileCoord * TILE_SIZE + threadID.xy;

    float minZ = NearZ;
    float maxZ = FarZ;

    // --- 1. 타일 내 각 픽셀의 Depth를 샘플링하고, 해당 슬라이스 인덱스의 비트를 그룹 공유 변수에 누적
    // 초기화: 그룹의 첫 번째 스레드가 tileDepthMask를 0으로 초기화
    if (threadID.x == 0 && threadID.y == 0)
    {
        tileDepthMask = 0;
        groupMinZ = 0x7f7fffff;
        groupMaxZ = 0x00000000; // 그룹공유 변수는 초기화 허용X, 쓰레기값 발생 가능
    }
    hitCount = 0;
    GroupMemoryBarrierWithGroupSync();

    float depthSample = 1.0f;
    float linearZ = FarZ;
    // (1) 만약 Enable25DCulling 옵션이 켜져 있다면, 해당 타일 내의 depth mask 구성
    if (Enable25DCulling != 0 && all(pixel < ScreenSize))
    {
        // 픽셀별 depth 샘플링
        depthSample = gDepthTexture[pixel];
        if (depthSample < 1.0f)
        {
            linearZ = (NearZ * FarZ) / (FarZ - depthSample * (FarZ - NearZ));
            uint linZ_uint = uint(linearZ);
            InterlockedMin(groupMinZ, linZ_uint);
            InterlockedMax(groupMaxZ, linZ_uint);
        }
    }

    GroupMemoryBarrierWithGroupSync();

    if (Enable25DCulling != 0 && groupMaxZ > groupMinZ)
    {
        minZ = float(groupMinZ);
        maxZ = float(groupMaxZ);
    }

    if (Enable25DCulling != 0 && depthSample < 1.0f)
    {
        float rangeZ = maxZ - minZ;
        if (rangeZ < 1e-1) // 지오메트리 depth 구간 정할 depth Sample 값의 범위가 너무 작은 경우, 임의로 늘려줌 (Dithering 문제 완화)
        {
            minZ = NearZ;
            maxZ = FarZ;
            rangeZ = maxZ - minZ;
        }

        float sliceNormZ = saturate((linearZ - minZ) / rangeZ);
        int sliceIndex = clamp((int) floor(sliceNormZ * NUM_SLICES), 0, NUM_SLICES - 1);
        uint sliceBit = (1u << sliceIndex);
        InterlockedOr(tileDepthMask, sliceBit);
    }
    GroupMemoryBarrierWithGroupSync();

    // 뷰 공간 프러스텀 계산
    float2 tileMin = tileCoord * TileSize; // GroupID 기준 tile Min, Max
    float2 tileMax = tileMin + TileSize;

    // 8개 코너의 clip space 점을 inverse projection하여 view space로 변환
    float3 viewCorners[8];
    [unroll]
    for (uint i = 0; i < 4; ++i)
    {
        float2 uv = float2((i & 1) ? tileMax.x : tileMin.x, (i & 2) ? tileMax.y : tileMin.y) / ScreenSize;
        uv.y = 1.0 - uv.y; // y축 반전해야 카메라 위로 들어올렸을 때 사각형은 아래로 감
        float4 clipNear = float4(uv * 2.0 - 1.0, NearZ, 1.0);
        float4 clipFar = float4(uv * 2.0 - 1.0, FarZ, 1.0);
        viewCorners[i + 0] = mul(clipNear, InverseProjection).xyz / mul(clipNear, InverseProjection).w;
        viewCorners[i + 4] = mul(clipFar, InverseProjection).xyz / mul(clipFar, InverseProjection).w;
    }

    // 프러스텀 4개 면 생성
    Frustum frustum;
    frustum.planes[0] = ComputePlane(viewCorners[0], viewCorners[2], viewCorners[6]); // left
    frustum.planes[1] = ComputePlane(viewCorners[3], viewCorners[1], viewCorners[7]); // right
    frustum.planes[2] = ComputePlane(viewCorners[1], viewCorners[0], viewCorners[5]); // top
    frustum.planes[3] = ComputePlane(viewCorners[2], viewCorners[3], viewCorners[6]); // bottom

    // 타일 인덱스
    uint flatTileIndex = tileCoord.y * (ScreenSize.x / TileSize.x) + tileCoord.x;

    if (threadID.x == 0 && threadID.y == 0)
    {
        CullLightsForTile(frustum, minZ, maxZ, flatTileIndex); // 2.5D 컬링이 활성화 -> 각 라이트의 view space 깊이 범위를 구해 depth mask와 교차 검사
    }

    GroupMemoryBarrierWithGroupSync();

    // 히트맵 출력
    if (threadID.x == 0 && threadID.y == 0) {
        WriteHeatmap(tileCoord);
    }

    // SV_DispatchThreadID는 전체 화면상의 픽셀 좌표(전역 좌표)를 나타냅니다.
}
