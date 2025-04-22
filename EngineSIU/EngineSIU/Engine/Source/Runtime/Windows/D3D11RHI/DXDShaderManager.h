#pragma once
#define _TCHAR_DEFINED
#include <d3d11.h>
#include <d3dcompiler.h>
#include <filesystem>
#include <chrono>
#include "Container/Map.h"
#include "Container/Array.h"
#include "Container/Set.h"
#include <vector>

//#define Multi_Shader_Include // 중첩 헤더 파일 지원 플래그 (주석 해제시 재귀적으로 include 검사/갱신)
struct FVertexShaderData
{
	ID3DBlob* VertexShaderCSO;
	ID3D11VertexShader* VertexShader;
};

struct FShaderReloadInfo {
    std::wstring Key;
    std::wstring FilePath;
    std::string EntryPoint;
    bool IsVertexShader;
    std::vector<D3D_SHADER_MACRO> Defines;
    std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;

    FShaderReloadInfo() = default;
    FShaderReloadInfo(const std::wstring& InKey, const std::wstring& InFilePath,
        const std::string& InEntryPoint, bool bIsVS)
        : Key(InKey), FilePath(InFilePath), EntryPoint(InEntryPoint), IsVertexShader(bIsVS) {
    }
};

class FDXDShaderManager
{
public:
	FDXDShaderManager() = default;
	FDXDShaderManager(ID3D11Device* Device);

    ~FDXDShaderManager();

    // Hot Reload 관련 함수
	void ReleaseAllShader();
    void UpdateShaderIfOutdated(const std::wstring Key, const std::wstring FilePath, const std::string EntryPoint, bool IsVertexShader, const D3D_SHADER_MACRO * Defines = nullptr, const D3D11_INPUT_ELEMENT_DESC * Layout = nullptr, uint32 LayoutSize = 0);
    void RegisterShaderForReload(std::wstring Key, std::wstring FilePath, std::string EntryPoint, bool IsVertexShader, D3D_SHADER_MACRO* Defines = nullptr, D3D11_INPUT_ELEMENT_DESC* Layout = nullptr, uint32 LayoutSize = 0);
    void ReloadAllShaders();

    // Dependency Graph 관련 함수
    void BuildDependency(const FShaderReloadInfo& Info);
    bool IsOutdatedWithDependency(const FShaderReloadInfo& Info);
    void UpdateDependencyTimestamps();
private:
	ID3D11Device* DXDDevice;

public:
	HRESULT AddVertexShader(const std::wstring& Key, const std::wstring& FileName);
	HRESULT AddVertexShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint);
    HRESULT AddVertexShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* defines);
	HRESULT AddInputLayout(const std::wstring& Key, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize);
    HRESULT AddComputeShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint);
    HRESULT AddGeometryShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint);
	
	HRESULT AddVertexShaderAndInputLayout(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize);
    HRESULT AddVertexShaderAndInputLayout(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize, const D3D_SHADER_MACRO* defines);
	HRESULT AddPixelShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint);
    HRESULT AddPixelShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D_SHADER_MACRO* defines);
	ID3D11InputLayout* GetInputLayoutByKey(const std::wstring& Key) const;
	ID3D11VertexShader* GetVertexShaderByKey(const std::wstring& Key) const;
	ID3D11PixelShader* GetPixelShaderByKey(const std::wstring& Key) const;
    ID3D11ComputeShader* GetComputeShaderByKey(const std::wstring& Key);
    ID3D11GeometryShader* GetGeometryShaderByKey(const std::wstring& Key);

private:
	TMap<std::wstring, ID3D11InputLayout*> InputLayouts;
	TMap<std::wstring, ID3D11VertexShader*> VertexShaders;
	TMap<std::wstring, ID3D11PixelShader*> PixelShaders;
    TMap<std::wstring, ID3D11ComputeShader*> ComputeShaders;
    TMap<std::wstring, ID3D11GeometryShader*> GeometryShaders;

    TMap<std::wstring, std::filesystem::file_time_type> ShaderTimeStamps;
    TMap<std::wstring, TSet<std::wstring>> ShaderDependencyGraph;
    std::vector<FShaderReloadInfo> RegisteredShaders;
};

