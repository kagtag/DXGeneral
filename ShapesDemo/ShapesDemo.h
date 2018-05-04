#pragma once

#include "d3dApp.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"

class ShapesApp :public D3DApp
{
private:
	struct VertexType
	{
		XMFLOAT3 Pos;
		XMFLOAT4 Color;
	};

	//Constant buffer
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;

	};

public:
	ShapesApp(HINSTANCE hInstance);
	~ShapesApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	bool DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	bool BuildGeometryBuffers();
	
	bool BuildShader(WCHAR*, WCHAR*);
	bool SetShaderParameters(XMMATRIX, XMMATRIX, XMMATRIX,int,int,int);

	void RenderShader(int, int, int);

	void RenderBuffers();

private:
	ID3D11Buffer* m_shapeVB;
	ID3D11Buffer* m_shapeIB;

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;

	ID3D11InputLayout *m_inputLayout;

	ID3D11Buffer* m_matrixBuffer;
	
	//
	ID3D11RasterizerState* m_wireFrameRS;

	//define world transformation
	XMFLOAT4X4 m_sphereWorld[10];
	XMFLOAT4X4 m_cylWorld[10];
	XMFLOAT4X4 m_boxWorld;
	XMFLOAT4X4 m_gridWorld;
	XMFLOAT4X4 m_centerSphere;

	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;

	//for draw call
	int m_boxVertexOffset;
	int m_gridVertexOffset;
	int m_sphereVertexOffset;
	int m_cylinderVertexOffset;

	UINT m_boxIndexOffset;
	UINT m_gridIndexOffset;
	UINT m_sphereIndexOffset;
	UINT m_cylinderIndexOffset;

	UINT m_boxIndexCount;
	UINT m_gridIndexCount;
	UINT m_sphereIndexCount;
	UINT m_cylinderIndexCount;


	float m_theta;
	float m_phi;
	float m_radius;

	POINT m_lastMousePos;
};
