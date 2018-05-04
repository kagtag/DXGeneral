#pragma once

#include"d3dApp.h"
#include"GeometryGenerator.h"
#include"MathHelper.h"

class SkullApp :public D3DApp
{
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
	SkullApp(HINSTANCE);
	~SkullApp();

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
	bool SetShaderParameters(XMMATRIX, XMMATRIX, XMMATRIX, int, int, int);

	void RenderShader(int, int, int);

	void RenderBuffers();

private:
	ID3D11Buffer* m_skullVB;
	ID3D11Buffer* m_skullIB;

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;

	ID3D11InputLayout* m_inputLayout;

	ID3D11Buffer* m_matrixBuffer;

	int m_indexCount;

	ID3D11RasterizerState* m_wireframeRS;

	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;

	XMFLOAT4X4 m_world;

	float m_theta;
	float m_phi;
	float m_radius;

	POINT m_lastMousePos;
};