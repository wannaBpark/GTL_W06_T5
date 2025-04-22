#include "ShadowManager.h"
// --- 생성자 및 소멸자 ---

FShadowManager::FShadowManager()
{
    // 포인터 멤버들을 nullptr로 초기화 (헤더에서도 초기화했지만, 명시적으로)
    D3DDevice = nullptr;
    D3DContext = nullptr;
    SpotShadowArrayTexture = nullptr;
    SpotShadowArraySRV = nullptr;
    DirectionalShadowTexture = nullptr;
    DirectionalShadowSRV = nullptr;
    //ShadowSampler = nullptr;
    ShadowSamplerCmp = nullptr;
}

FShadowManager::~FShadowManager()
{
    // 소멸 시 자동으로 리소스 해제 호출
    Release();
}

// --- Public 멤버 함수 구현 ---

bool FShadowManager::Initialize(ID3D11Device* InDevice, ID3D11DeviceContext* InContext,
                               uint32_t InMaxSpotShadows, uint32_t InSpotResolution,
                               uint32_t InNumCascades, uint32_t InDirResolution)
{
    // 이미 초기화되었으면 먼저 해제
    if (D3DDevice)
    {
        Release();
    }

    D3DDevice = InDevice;
    D3DContext = InContext;

    // 설정 값 저장
    MaxSpotLightShadows = InMaxSpotShadows;
    SpotShadowMapResolution = InSpotResolution;
    NumCascades = InNumCascades;
    DirectionalShadowMapResolution = InDirResolution;

    // 리소스 생성 시도
    if (!CreateSpotShadowResources())
    {
        // UE_LOG(LogTemp, Error, TEXT("Failed to create spot shadow resources!"));
        Release(); // 실패 시 생성된 것들 정리
        return false;
    }
    if (!CreateDirectionalShadowResources())
    {
        // UE_LOG(LogTemp, Error, TEXT("Failed to create directional shadow resources!"));
        Release();
        return false;
    }
    if (!CreateSamplers())
    {
        // UE_LOG(LogTemp, Error, TEXT("Failed to create shadow samplers!"));
        Release();
        return false;
    }

    // 방향성 광원 ViewProj 행렬 배열 크기 설정
    DirectionalLightViewProjMatrices.SetNum(NumCascades);

    // UE_LOG(LogTemp, Log, TEXT("FShadowManager Initialized Successfully."));
    return true;
}

void FShadowManager::Release()
{
    // 생성된 역순 또는 그룹별로 리소스 해제
    ReleaseSamplers();
    ReleaseDirectionalShadowResources();
    ReleaseSpotShadowResources();

    // 배열 클리어
    DirectionalLightViewProjMatrices.Empty();

    // D3D 객체 포인터는 외부에서 관리하므로 여기서는 nullptr 처리만 함
    D3DDevice = nullptr;
    D3DContext = nullptr;
}

void FShadowManager::BeginSpotShadowPass(uint32_t sliceIndex)
{
    // 유효성 검사
    if (!D3DContext || sliceIndex >= (uint32_t)SpotShadowSliceDSVs.Num() || !SpotShadowSliceDSVs[sliceIndex])
    {
        // UE_LOG(LogTemp, Warning, TEXT("BeginSpotShadowPass: Invalid slice index or DSV."));
        return;
    }

    // 렌더 타겟 설정 (DSV만 설정)
    ID3D11RenderTargetView* nullRTV = nullptr;
    D3DContext->OMSetRenderTargets(1, &nullRTV, SpotShadowSliceDSVs[sliceIndex]);

    // 뷰포트 설정
    D3D11_VIEWPORT vp = {};
    vp.Width = (float)SpotShadowMapResolution;
    vp.Height = (float)SpotShadowMapResolution;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    D3DContext->RSSetViewports(1, &vp);

    // DSV 클리어
    D3DContext->ClearDepthStencilView(SpotShadowSliceDSVs[sliceIndex], D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void FShadowManager::BeginDirectionalShadowCascadePass(uint32_t cascadeIndex)
{
    // 유효성 검사
    if (!D3DContext || cascadeIndex >= (uint32_t)DirectionalShadowCascadeDSVs.Num() || !DirectionalShadowCascadeDSVs[cascadeIndex])
    {
        // UE_LOG(LogTemp, Warning, TEXT("BeginDirectionalShadowCascadePass: Invalid cascade index or DSV."));
        return;
    }

    // 렌더 타겟 설정 (DSV만 설정)
    ID3D11RenderTargetView* nullRTV = nullptr;
    D3DContext->OMSetRenderTargets(1, &nullRTV, DirectionalShadowCascadeDSVs[cascadeIndex]);

    // 뷰포트 설정
    D3D11_VIEWPORT vp = {};
    vp.Width = (float)DirectionalShadowMapResolution;
    vp.Height = (float)DirectionalShadowMapResolution;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    D3DContext->RSSetViewports(1, &vp);

    // DSV 클리어
    D3DContext->ClearDepthStencilView(DirectionalShadowCascadeDSVs[cascadeIndex], D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void FShadowManager::BindResourcesForSampling(uint32_t spotShadowSlot, uint32_t directionalShadowSlot, uint32_t samplerCmpSlot)
{
    if (!D3DContext) return;

    //ID3D11ShaderResourceView* psSRVs[] = { nullptr, nullptr }; // 슬롯에 맞게 크기 조절 필요
    //if (spotShadowSlot < ARRAYSIZE(psSRVs)) psSRVs[spotShadowSlot] = SpotShadowArraySRV; // 슬롯 번호 직접 사용보다는 맵 등으로 관리하는 것이 안전
    //if (directionalShadowSlot < ARRAYSIZE(psSRVs)) psSRVs[directionalShadowSlot] = DirectionalShadowSRV;

    // 실제 사용할 슬롯 범위에 맞춰 SRV 설정
    // 예시: 10번, 11번 슬롯 사용 시
    // ID3D11ShaderResourceView* psSRVs[2] = { SpotShadowArraySRV, DirectionalShadowSRV };
    // D3DContext->PSSetShaderResources(spotShadowSlot, 2, psSRVs); // 시작 슬롯과 개수 사용

    // 개별 바인딩 
    if (SpotShadowArraySRV)
    {
        D3DContext->PSSetShaderResources(spotShadowSlot, 1, &SpotShadowArraySRV);
    }
    if (DirectionalShadowSRV)
    {
        D3DContext->PSSetShaderResources(directionalShadowSlot, 1, &DirectionalShadowSRV);
    }


    //ID3D11SamplerState* psSamplers[] = { nullptr };
    //if (samplerCmpSlot < ARRAYSIZE(psSamplers)) psSamplers[samplerCmpSlot] = ShadowSamplerCmp;
    // D3DContext->PSSetSamplers(samplerCmpSlot, 1, psSamplers); // 시작 슬롯과 개수 사용

    // 개별 바인딩
    if (ShadowSamplerCmp)
    {
        D3DContext->PSSetSamplers(samplerCmpSlot, 1, &ShadowSamplerCmp);
    }
    // 필요시 다른 샘플러도 바인딩
}


// --- Private 멤버 함수 구현 (리소스 생성/해제 헬퍼) ---

bool FShadowManager::CreateSpotShadowResources()
{
    // 유효성 검사
    if (!D3DDevice || MaxSpotLightShadows == 0 || SpotShadowMapResolution == 0) return false;

    // 1. Texture2DArray 생성
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = SpotShadowMapResolution;
    texDesc.Height = SpotShadowMapResolution;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = MaxSpotLightShadows;
    texDesc.Format = DXGI_FORMAT_D32_FLOAT; // 깊이 포맷
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    HRESULT hr = D3DDevice->CreateTexture2D(&texDesc, nullptr, &SpotShadowArrayTexture);
    if (FAILED(hr))
    {
        // UE_LOG(LogTemp, Error, TEXT("CreateTexture2D failed for SpotShadowArrayTexture (HR=0x%X)"), hr);
        return false;
    }

    // 2. 전체 배열용 SRV 생성
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT; // 읽기용 포맷
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MostDetailedMip = 0;
    srvDesc.Texture2DArray.MipLevels = 1;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.ArraySize = MaxSpotLightShadows;

    hr = D3DDevice->CreateShaderResourceView(SpotShadowArrayTexture, &srvDesc, &SpotShadowArraySRV);
    if (FAILED(hr))
    {
        // UE_LOG(LogTemp, Error, TEXT("CreateShaderResourceView failed for SpotShadowArraySRV (HR=0x%X)"), hr);
        ReleaseSpotShadowResources(); // 생성된 텍스처 정리
        return false;
    }

    // 3. 각 슬라이스용 DSV 생성
    SpotShadowSliceDSVs.SetNum(MaxSpotLightShadows); // 배열 크기 예약
    for (uint32_t i = 0; i < MaxSpotLightShadows; ++i)
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = texDesc.Format; // 깊이 포맷
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        dsvDesc.Texture2DArray.MipSlice = 0;
        dsvDesc.Texture2DArray.FirstArraySlice = i;
        dsvDesc.Texture2DArray.ArraySize = 1;
        dsvDesc.Flags = 0;

        hr = D3DDevice->CreateDepthStencilView(SpotShadowArrayTexture, &dsvDesc, &SpotShadowSliceDSVs[i]);
        if (FAILED(hr))
        {
            // UE_LOG(LogTemp, Error, TEXT("CreateDepthStencilView failed for SpotShadowSliceDSV[%u] (HR=0x%X)"), i, hr);
            ReleaseSpotShadowResources(); // 생성된 것들 정리
            return false;
        }
    }

    return true;
}

void FShadowManager::ReleaseSpotShadowResources()
{
    // 생성된 DSV 해제
    for (auto& dsv : SpotShadowSliceDSVs)
    {
        if (dsv)
        {
            dsv->Release();
            dsv = nullptr; // 포인터 초기화
        }
    }
    SpotShadowSliceDSVs.Empty(); // 배열 비우기

    // SRV 해제
    if (SpotShadowArraySRV)
    {
        SpotShadowArraySRV->Release();
        SpotShadowArraySRV = nullptr;
    }
    // 텍스처 해제
    if (SpotShadowArrayTexture)
    {
        SpotShadowArrayTexture->Release();
        SpotShadowArrayTexture = nullptr;
    }
}

bool FShadowManager::CreateDirectionalShadowResources()
{
    // 유효성 검사
    if (!D3DDevice || NumCascades == 0 || DirectionalShadowMapResolution == 0) return false;

    // 1. Texture2DArray 생성 (CSM 용)
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = DirectionalShadowMapResolution;
    texDesc.Height = DirectionalShadowMapResolution;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = NumCascades; // 캐스케이드 개수만큼
    texDesc.Format = DXGI_FORMAT_D32_FLOAT;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = D3DDevice->CreateTexture2D(&texDesc, nullptr, &DirectionalShadowTexture);
    if (FAILED(hr)) return false;

    // 2. 전체 배열용 SRV 생성
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MostDetailedMip = 0;
    srvDesc.Texture2DArray.MipLevels = 1;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.ArraySize = NumCascades;

    hr = D3DDevice->CreateShaderResourceView(DirectionalShadowTexture, &srvDesc, &DirectionalShadowSRV);
    if (FAILED(hr)) { ReleaseDirectionalShadowResources(); return false; }

    // 3. 각 캐스케이드용 DSV 생성
    DirectionalShadowCascadeDSVs.SetNum(NumCascades);
    for (uint32_t i = 0; i < NumCascades; ++i)
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = texDesc.Format;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        dsvDesc.Texture2DArray.MipSlice = 0;
        dsvDesc.Texture2DArray.FirstArraySlice = i;
        dsvDesc.Texture2DArray.ArraySize = 1;

        hr = D3DDevice->CreateDepthStencilView(DirectionalShadowTexture, &dsvDesc, &DirectionalShadowCascadeDSVs[i]);
        if (FAILED(hr)) { ReleaseDirectionalShadowResources(); return false; }
    }
    return true;
}

void FShadowManager::ReleaseDirectionalShadowResources()
{
    for (auto& dsv : DirectionalShadowCascadeDSVs)
    {
        if (dsv) { dsv->Release(); dsv = nullptr; }
    }
    DirectionalShadowCascadeDSVs.Empty();

    if (DirectionalShadowSRV) { DirectionalShadowSRV->Release(); DirectionalShadowSRV = nullptr; }
    if (DirectionalShadowTexture) { DirectionalShadowTexture->Release(); DirectionalShadowTexture = nullptr; }
}

bool FShadowManager::CreateSamplers()
{
    if (!D3DDevice) return false;

    // 1. Comparison Sampler (PCF 용)
    D3D11_SAMPLER_DESC sampDescCmp = {};
    sampDescCmp.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT; // 선형 필터링 비교
    sampDescCmp.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDescCmp.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDescCmp.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDescCmp.MipLODBias = 0.0f;
    sampDescCmp.MaxAnisotropy = 1;
    sampDescCmp.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL; // 깊이 비교 함수
    sampDescCmp.BorderColor[0] = 1.0f; // 경계 밖 = 깊이 최대 (그림자 없음)
    sampDescCmp.BorderColor[1] = 1.0f;
    sampDescCmp.BorderColor[2] = 1.0f;
    sampDescCmp.BorderColor[3] = 1.0f;
    sampDescCmp.MinLOD = 0;
    sampDescCmp.MaxLOD = D3D11_FLOAT32_MAX;

    HRESULT hr = D3DDevice->CreateSamplerState(&sampDescCmp, &ShadowSamplerCmp);
    if (FAILED(hr)) return false;

    // 필요시 VSM/ESM 등을 위한 일반 샘플러(ShadowSampler) 생성

    return true;
}

void FShadowManager::ReleaseSamplers()
{
    if (ShadowSamplerCmp) { ShadowSamplerCmp->Release(); ShadowSamplerCmp = nullptr; }
    //if (ShadowSampler) { ShadowSampler->Release(); ShadowSampler = nullptr; }
}
