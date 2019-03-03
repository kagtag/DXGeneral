#pragma once

#include "d3dUtil.h"

#include "TerVertex.h"
#include "TerEffects.h"

class Camera;
struct DirectionalLight;

class Terrain
{
public:
	struct InitInfo
	{
		std::wstring HeightMapFilename;
		
		std::wstring LayerMapFilename0;
		std::wstring LayerMapFilename1;
		std::wstring LayerMapFilename2;
		std::wstring LayerMapFilename3;
		std::wstring LayerMapFilename4;

		std::wstring BlendMapFilename;

		float HeightScale;
		UINT HeightmapWidth;
		UINT HeightmapHeight;

		float CellSpacing;
	};

public:
	Terrain();
	~Terrain();

	float GetWidth()const;
	float GetDepth()const;
	float GetHeight(float x, float z)const;

	XMMATRIX GetWorld()const;
	void SetWorld(CXMMATRIX M);

	void Init(ID3D11Device* device, ID3D11DeviceContext* dc, const InitInfo& initInfo);
	
	void Draw(ID3D11DeviceContext* dx, const Camera& cam, DirectionalLight lights[3], 
		ID3D11InputLayout* layout, TerrainEffect* effect);

private:
	void LoadHeightMap();
	void Smooth();
	bool InBounds(int i, int j);
	float Average(int i, int j);

	void CalcAllPatchBoundsY();
	void CalcPatchBoundsY(UINT i, UINT j);
	
	void BuildQuadPatchVB(ID3D11Device* device);
	void BuildQuadPatchIB(ID3D11Device* device);
	void BuildHeightMapSRV(ID3D11Device* device);

private:
	
	// Divide heightmap into patches such that each patch has CellsPerPatch cells
	// and CellsPerPatch+1 vertices. Use 64 so that if we tessellate all the way to 
	// 64, we use all the data from the heightmap

	static const int CellsPerPatch = 64;

	ID3D11Buffer* mQuadPatchVB;
	ID3D11Buffer* mQuadPatchIB;

	ID3D11ShaderResourceView* mLayerMapArraySRV;
	ID3D11ShaderResourceView* mBlendMapSRV;
	ID3D11ShaderResourceView* mHeightMapSRV;

	InitInfo mInfo;

	UINT mNumPatchVertices;
	UINT mNumPatchQuadFaces;

	UINT mNumPatchVertRows;
	UINT mNumPatchVertCols;

	XMFLOAT4X4 mWorld;

	Material mMat;

	std::vector<XMFLOAT2> mPatchBoundsY;
	std::vector<float> mHeightmap;
};