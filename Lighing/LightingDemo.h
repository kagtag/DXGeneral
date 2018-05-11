#pragma once
#include"d3dApp.h"
#include"GeometryGenerator.h"
#include"MathHelper.h"
#include"LightHelper.h"
#include"Waves.h"

class LightingApp : public D3DApp
{
private:
	struct VertexType
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
	};


	//Vertex constant buffers.
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;

		XMMATRIX wInvTrans;
	};

	struct EyeBufferType
	{
		XMFLOAT3 eye;
		float pad;
	};

	//Pixel constant buffer
	//Color of lights, color of objects
	struct LightBufferType
	{
		DirectionalLight dir;
		PointLight point;
		SpotLight spot;

		Material mat;
	};



public:
	LightingApp(HINSTANCE hInstance);
	~LightingApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	bool DrawScene();

	void OnMouseDown(WPARAM, int, int);
	void OnMouseUp(WPARAM, int, int);
	void OnMouseMove(WPARAM, int, int);

private:
	float GetHillHeight(float x, float z)const;
	XMFLOAT3 GetHillNormal(float x, float z)const;
	void BuildLandGeometryBuffers();
	void BuildWaveGeometryBuffers();

	bool BuildShader(WCHAR*, WCHAR*);
	bool SetShaderParameters(XMMATRIX, XMMATRIX, XMMATRIX, XMMATRIX, Material, int, int, int);
	void RenderShader(int, int, int);

	void RenderBuffers();

private:
	ID3D11Buffer* m_landVB;
	ID3D11Buffer* m_landIB;

	ID3D11Buffer* m_wavesVB;
	ID3D11Buffer* m_wavesIB;
	
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;

	ID3D11InputLayout* m_inputLayout;

	//constant buffers
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_lightBuffer;
	ID3D11Buffer* m_eyeBuffer;

	ID3D11RasterizerState* m_wireframeRS;

	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;

	XMFLOAT4X4 m_landWorld;
	XMFLOAT4X4 m_wavesWorld;

	UINT m_landIndexCount;
	
	//
	Waves m_waves;

	float m_theta;
	float m_phi;
	float m_radius;

	POINT m_lastMousePos;

	//
	XMFLOAT4X4 m_worldInvTranspose;
	XMFLOAT3 m_eyePosW;

	DirectionalLight m_dirLight;
	PointLight m_pointLight;
	SpotLight m_spotLight;

	Material m_landMat;
	Material m_wavesMat;

};