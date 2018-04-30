#pragma once

//***************************************************************************************
// BoxDemo.cpp by Frank Luna (C) 2011 All Rights Reserved.
//
// Demonstrates rendering a colored box.
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//
//***************************************************************************************

#include"d3dApp.h"
#include"MathHelper.h"




class BoxApp :public D3DApp
{
private:

	//Vertex Shader
	struct Vertex
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
	BoxApp(HINSTANCE hInstance);
	~BoxApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	bool DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	bool BuildGeometryBuffers();
	//void BuildFX();
	//void BuildVertexLayout();
	bool BuildShader(WCHAR*, WCHAR*); //take in vs and ps filename

	bool SetShaderParameters(XMMATRIX, XMMATRIX, XMMATRIX);

	//
	void RenderShader(int);

	void RenderBuffers(); //model class

private:
	ID3D11Buffer* m_boxVB;
	ID3D11Buffer* m_boxIB;

	//No effect or technique here

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;

	ID3D11InputLayout* m_inputLayout;

	ID3D11Buffer* m_matrixBuffer; //constant buffer

	XMFLOAT4X4 m_world;
	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;

	float m_theta;
	float m_phi;
	float m_radius;

	POINT m_lastMousePos;

	//
	int m_indexCount; //number of vertex in the model
	//bool m_vsync_enabled; //lock refresh rate or not
};
