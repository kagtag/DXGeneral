#pragma once
#include"d3dApp.h"
#include"GeometryGenerator.h"
#include"MathHelper.h"
#include"Waves.h"

class WavesApp :public D3DApp
{
private:
	struct VertexType
	{
		XMFLOAT3 Pos;
		XMFLOAT4 Color;
	};

	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

public:
	WavesApp(HINSTANCE hInstance);
	~WavesApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	bool DrawScene();

	void OnMouseDown(WPARAM, int, int);
	void OnMouseUp(WPARAM, int, int);
	void OnMouseMove(WPARAM, int, int);

private:
	float GetHeight(float x, float z)const;
	void BuildLandGeometryBuffers();
	void BuildWavesGeometryBuffers();

	bool BuildShader(WCHAR*, WCHAR*);
	bool SetShaderParameters(XMMATRIX, XMMATRIX, XMMATRIX, int, int, int);
	void RenderShader(int, int, int);

	//void RenderBuffers();

private:
	ID3D11Buffer* m_landVB;
	ID3D11Buffer* m_landIB;

	ID3D11Buffer* m_wavesVB;
	ID3D11Buffer* m_wavesIB;
	
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;

	ID3D11InputLayout* m_inputLayout;
	ID3D11Buffer* m_matrixBuffer;

	ID3D11RasterizerState* m_wireframeRS;

	XMFLOAT4X4 m_gridWorld;
	XMFLOAT4X4 m_wavesWorld;

	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;

	UINT m_gridIndexCount;
	
	Waves m_waves;

	float m_theta;
	float m_phi;
	float m_radius;

	POINT m_lastMousePos;

};