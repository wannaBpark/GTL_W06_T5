#pragma once

#include <d3d11.h>

#include "RendererHelpers.h"
#include "Container/Array.h"
#include "Math/Matrix.h"     // FMatrix (UE 스타일)

class FShadowManager
{
public:


    // --- 생성자 및 소멸자 ---
    FShadowManager();
    ~FShadowManager();

    // --- Public 멤버 함수 ---

    /**
     * 섀도우 매니저를 초기화하고 필요한 D3D 리소스를 생성합니다.
     * @param InDevice D3D11 디바이스 포인터
     * @param InContext D3D11 디바이스 컨텍스트 포인터
     * @param InMaxSpotShadows 지원할 최대 스포트라이트 섀도우 개수
     * @param InSpotResolution 스포트라이트 섀도우 맵 해상도
     * @param InNumCascades 방향성 광원 CSM 캐스케이드 개수
     * @param InDirResolution 방향성 광원 섀도우 맵 해상도
     * @return 초기화 성공 여부
     */
    bool Initialize(ID3D11Device* InDevice, ID3D11DeviceContext* InContext,
                    uint32_t InMaxSpotShadows = 16, uint32_t InSpotResolution = 1024,
                    uint32_t InNumCascades = 1, uint32_t InDirResolution = 2048);

    /** 생성된 모든 D3D 리소스를 해제합니다. */
    void Release();

    /**
     * 특정 스포트라이트 섀도우 맵 렌더링 패스를 시작하기 위해 DSV와 뷰포트를 설정하고 클리어합니다.
     * @param sliceIndex 렌더링할 Texture2DArray의 슬라이스 인덱스
     */
    void BeginSpotShadowPass(uint32_t sliceIndex);

    /**
     * 특정 방향성 광원 캐스케이드 섀도우 맵 렌더링 패스를 시작하기 위해 DSV와 뷰포트를 설정하고 클리어합니다.
     * @param cascadeIndex 렌더링할 캐스케이드 인덱스
     */
    void BeginDirectionalShadowCascadePass(uint32_t cascadeIndex);

    /**
     * 메인 렌더링 패스에서 픽셀 셰이더가 섀도우 맵을 샘플링할 수 있도록 관련 리소스(SRV, 샘플러)를 바인딩합니다.
     * @param spotShadowSlot 스포트라이트 섀도우 맵 SRV를 바인딩할 PS 텍스처 슬롯 번호
     * @param directionalShadowSlot 방향성 광원 섀도우 맵 SRV를 바인딩할 PS 텍스처 슬롯 번호
     * @param samplerCmpSlot 비교 샘플러를 바인딩할 PS 샘플러 슬롯 번호
     */
    void BindResourcesForSampling(uint32_t spotShadowSlot = static_cast<UINT>(EShaderSRVSlot::SRV_SpotLight),
        uint32_t directionalShadowSlot = static_cast<UINT>(EShaderSRVSlot::SRV_DirectionalLight), uint32_t samplerCmpSlot = 10);


private:
    
    // D3D 디바이스 및 컨텍스트 (외부에서 설정)
    ID3D11Device* D3DDevice = nullptr;
    ID3D11DeviceContext* D3DContext = nullptr;

    // --- 스포트라이트 섀도우 리소스 ---
    ID3D11Texture2D* SpotShadowArrayTexture = nullptr;
    ID3D11ShaderResourceView* SpotShadowArraySRV = nullptr;
    TArray<ID3D11DepthStencilView*> SpotShadowSliceDSVs;

    uint32 MaxSpotLightShadows = 16;
    uint32 SpotShadowMapResolution = 1024;

    // --- 방향성 광원 섀도우 리소스 (CSM 기반) ---
    ID3D11Texture2D* DirectionalShadowTexture = nullptr;
    ID3D11ShaderResourceView* DirectionalShadowSRV = nullptr;
    
    
    TArray<ID3D11DepthStencilView*> DirectionalShadowCascadeDSVs;

    uint32 MaxDirectionalLightShadows = 1; 
    uint32 NumCascades = 1;
    uint32 DirectionalShadowMapResolution = 2048;
    
    TArray<FMatrix> DirectionalLightViewProjMatrices; // 계산된 캐스케이드 ViewProj 행렬

    // --- 공통 샘플러 ---
    ID3D11SamplerState* ShadowSamplerCmp = nullptr;
    
    // --- Private 멤버 함수 (리소스 생성/해제 헬퍼) ---

    /** 스포트라이트 섀도우 관련 D3D 리소스(텍스처, SRV, DSV 배열)를 생성합니다. */
    bool CreateSpotShadowResources();
    /** 스포트라이트 섀도우 관련 D3D 리소스를 해제합니다. */
    void ReleaseSpotShadowResources();

    /** 방향성 광원 섀도우 관련 D3D 리소스(텍스처, SRV, DSV 배열)를 생성합니다. */
    bool CreateDirectionalShadowResources();
    /** 방향성 광원 섀도우 관련 D3D 리소스를 해제합니다. */
    void ReleaseDirectionalShadowResources();

    /** 섀도우 샘플링에 사용될 D3D 샘플러 상태(비교 샘플러 등)를 생성합니다. */
    bool CreateSamplers();
    /** 생성된 D3D 샘플러 상태를 해제합니다. */
    void ReleaseSamplers();
    
};
