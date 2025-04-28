#include "CameraComponent.h"
#include "Math/JungleMath.h"

FVector UCameraComponent::GetForwardVector() const
{
    FVector Forward = FVector(1.f, 0.f, 0.0f);
    Forward = JungleMath::FVectorRotate(Forward, ViewRotation);
    return Forward;
}

FVector UCameraComponent::GetRightVector() const
{
    FVector Right = FVector(0.f, 1.f, 0.0f);
    Right = JungleMath::FVectorRotate(Right, ViewRotation);
    return Right;
}

FVector UCameraComponent::GetUpVector() const
{
    FVector Up = FVector(0.f, 0.f, 1.0f);
    Up = JungleMath::FVectorRotate(Up, ViewRotation);
    return Up;
}

void UCameraComponent::UpdateViewMatrix()
{
    if (IsPerspective())
    {
        View = JungleMath::CreateViewMatrix(GetLocation(),
            GetLocation() + GetForwardVector(),
            FVector{ 0.0f, 0.0f, 1.0f }
        );
    }
    //else
    //{
    //    UpdateOrthoCameraLoc();
    //    if (ViewportType == LVT_OrthoXY || ViewportType == LVT_OrthoNegativeXY)
    //    {
    //        View = JungleMath::CreateViewMatrix(OrthogonalCamera.GetLocation(),
    //            Pivot, FVector(0.0f, -1.0f, 0.0f)
    //        );
    //    }
    //    else
    //    {
    //        View = JungleMath::CreateViewMatrix(OrthogonalCamera.GetLocation(),
    //            Pivot, FVector(0.0f, 0.0f, 1.0f)
    //        );
    //    }
    //}
}

void UCameraComponent::UpdateProjectionMatrix()
{
    if (IsPerspective())
    {
        Projection = JungleMath::CreateProjectionMatrix(
            FMath::DegreesToRadians(ViewFOV),
            AspectRatio,
            NearClip,
            FarClip
        );
    }
    else
    {
        // 오쏘그래픽 너비는 줌 값과 가로세로 비율에 따라 결정됩니다.
        const float OrthoWidth = OrthoSize * AspectRatio;
        const float OrthoHeight = OrthoSize;

        // 오쏘그래픽 투영 행렬 생성 (nearPlane, farPlane 은 기존 값 사용)
        Projection = JungleMath::CreateOrthoProjectionMatrix(
            OrthoWidth,
            OrthoHeight,
            NearClip,
            FarClip
        );
    }
}

void UCameraComponent::UpdateOrthoCameraLoc()
{
}

bool UCameraComponent::IsOrthographic() const
{
    return !IsPerspective();
}

bool UCameraComponent::IsPerspective() const
{
    return true;
}
