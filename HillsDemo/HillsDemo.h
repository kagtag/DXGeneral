#pragma once

#include "d3dApp.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"

class HillsApp:public D3DApp
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
	HillsApp(HINSTANCE hInstance);
	~HillsApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	bool DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	float GetHeight(float x, float y)const;
	
	bool BuildGeometryBuffers();

	bool BuildShader(WCHAR*, WCHAR*);
	bool SetShaderParameters(XMMATRIX, XMMATRIX, XMMATRIX);

	void RenderShader(int);

	void RenderBuffers();

private:
	ID3D11Buffer* m_hillVB;
	ID3D11Buffer* m_hillIB;
	
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;

	ID3D11InputLayout* m_inputLayout;

	ID3D11Buffer* m_matrixBuffer; //constant buffer

	XMFLOAT4X4 m_world;
	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_projection;

	int m_indexCount;

	float m_theta;
	float m_phi;
	float m_radius;

	POINT m_lastMousePos;

};