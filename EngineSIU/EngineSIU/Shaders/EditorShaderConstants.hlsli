struct AABBData
{
    float3 Position;
    float Padding1;
    
    float3 Extent;
    float Padding2;
};

cbuffer ConstantBufferDebugAABB : register(b11)
{
    AABBData DataAABB[8];
}

struct SphereData
{
    float3 Position;
    float Radius;
};

cbuffer ConstantBufferDebugSphere : register(b11)
{
    SphereData DataSphere[8];
}

struct ConeData
{
    float3 ApexPosiiton;
    float Radius;

    float3 Direction;
    float Angle;
};

cbuffer ConstantBufferDebugCone : register(b11)
{
    ConeData DataCone[100];
}

cbuffer ConstantBufferDebugGrid : register(b11)
{
    row_major matrix InverseViewProj;
}

cbuffer ConstantBufferDebugIcon : register(b11)
{
    float3 IconPosition;
    float IconScale;
}

struct ArrowData
{
    float3 Position;
    float  Scale;
    float3 Direction;
    float  ScaleZ;
};

cbuffer ConstantBufferDebugArrow : register(b11)
{
    ArrowData DataArrow[100];
}
