#include "AppHelper.h"

#include "NorDisEffects.h"
#include "NorDisVertex.h"

#include "Sky.h"

#include "Vertex.h"


enum RenderOptions
{
	RenderOptionsBasic = 0,
	RenderOptionsNormalMap = 1,
	RenderOptionsDisplacementMap = 2,
};


class NormalDisplacementMapApp : public ShapesBaseApp
{
public:
	NormalDisplacementMapApp(HINSTANCE hInstance);
	~NormalDisplacementMapApp();

	bool Init();
	void UpdateScene(float dt);
	bool DrawScene();

	virtual void BuildShapeGeometryBuffers() override;

private:
	void DrawTextured();


private:
	
	static bool wireframeMode;

	Sky* mSky;

	ID3D11ShaderResourceView* mStoneNormalTexSRV;
	ID3D11ShaderResourceView* mBrickNormalTexSRV;

	RenderOptions mRenderOptions;

};

DEFAULT_WINMAIN(NormalDisplacementMapApp)

bool NormalDisplacementMapApp::wireframeMode = false;

NormalDisplacementMapApp::NormalDisplacementMapApp(HINSTANCE hInstance)
	:ShapesBaseApp(hInstance),
	mSky(0),
	mStoneNormalTexSRV(0), mBrickNormalTexSRV(0),
	mRenderOptions(RenderOptionsNormalMap)
{
	m_mainWndCaption = L"Normal-Displacement Map Demo";

	XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&mSkullWorld, XMMatrixMultiply(skullScale, skullOffset));


}

NormalDisplacementMapApp::~NormalDisplacementMapApp()
{
	SafeDelete(mSky);

	ReleaseCOM(mStoneNormalTexSRV);
	ReleaseCOM(mBrickNormalTexSRV);

	NorDisEffects::DestroyAll();
	NorDisInputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool NormalDisplacementMapApp::Init()
{
	if (!ShapesBaseApp::Init())
		return false;

	NorDisEffects::InitAll(m_d3dDevice);
	NorDisInputLayouts::InitAll(m_d3dDevice);
	RenderStates::InitAll(m_d3dDevice);

	mSky = new Sky(m_d3dDevice, L"../Textures/snowcube1024.dds", 5000.0f);

	HR(CreateDDSTextureFromFile(m_d3dDevice, L"../Textures/floor_nmap.dds", 0, &mStoneNormalTexSRV));

	HR(CreateDDSTextureFromFile(m_d3dDevice, L"../Textures/bricks_nmap.dds", 0, &mBrickNormalTexSRV));

	return true;
}

void NormalDisplacementMapApp::UpdateScene(float dt)
{
	// Control the camera
	ShapesBaseApp::UpdateScene(dt);

	// Switch the render effect based on key presses
	if (GetAsyncKeyState('2') & 0x8000)
		mRenderOptions = RenderOptionsBasic;

	if (GetAsyncKeyState('3') & 0x8000)
		mRenderOptions = RenderOptionsNormalMap;

	if (GetAsyncKeyState('4') & 0x8000)
		mRenderOptions = RenderOptionsDisplacementMap;
}

bool NormalDisplacementMapApp::DrawScene()
{
	PreProcessing();

	mCam.UpdateViewMatrix(); // ??? why here

	XMMATRIX view = mCam.View();
	XMMATRIX proj = mCam.Proj();
	XMMATRIX viewProj = mCam.ViewProj();

	float blendFactor[] = { 0.0f,0.0f, 0.0f, 0.0f };

	// Set per frame constants
	NorDisEffects::BasicFX->SetDirLights(mDirLights);
	NorDisEffects::BasicFX->SetEyePosW(mCam.GetPosition());
	NorDisEffects::BasicFX->SetCubeMap(mSky->CubeMapSRV());

	NorDisEffects::NormalMapFX->SetDirLights(mDirLights);
	NorDisEffects::NormalMapFX->SetEyePosW(mCam.GetPosition());
	NorDisEffects::NormalMapFX->SetCubeMap(mSky->CubeMapSRV());

	NorDisEffects::DisplacementMapFX->SetDirLights(mDirLights);
	NorDisEffects::DisplacementMapFX->SetEyePosW(mCam.GetPosition());
	NorDisEffects::DisplacementMapFX->SetCubeMap(mSky->CubeMapSRV());


	NorDisEffects::DisplacementMapFX->SetHeightScale(0.07f);
	NorDisEffects::DisplacementMapFX->SetMinTessDistance(15.0f);
	NorDisEffects::DisplacementMapFX->SetMaxTessDistance(1.0f);
	NorDisEffects::DisplacementMapFX->SetMinTessFactor(1.0f);
	NorDisEffects::DisplacementMapFX->SetMaxTessFactor(5.0f);

	// Figure out which technique to use for different geometry

	ID3DX11EffectTechnique* activeSphereTech = NorDisEffects::BasicFX->Light3ReflectTech;
	ID3DX11EffectTechnique* activeSkullTech = NorDisEffects::BasicFX->Light3ReflectTech;





	// Draw the grid, cylinders, and box without any cubemap reflection

	UINT stride = sizeof(Vertex::PosNormalTexTan);
	UINT offset = 0;

	m_d3dImmediateContext->IASetInputLayout(NorDisInputLayouts::PosNormalTexTan);
	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
	m_d3dImmediateContext->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

	

	if (GetAsyncKeyState('1') & 0x8000)
	{
			wireframeMode = !wireframeMode;
	}

	if (wireframeMode)
	{
		m_d3dImmediateContext->RSSetState(RenderStates::WireframeRS);
	}
	
	// Draw textured objects
	DrawTextured();

	//
	// FX sets tessellation states, but it does not disable them. So do that 
	// here to turn off tessellation
	//
	m_d3dImmediateContext->HSSetShader(0, 0, 0);
	m_d3dImmediateContext->DSSetShader(0, 0, 0);

	m_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//
	// Draw the spheres with cubemap reflection
	//

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeSphereTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw the shperes
		for (int i = 0; i < 10; ++i)
		{
			MATRICES_SET(NorDisEffects::BasicFX, mSphereWorld[i])

			NorDisEffects::BasicFX->SetTexTransform(XMMatrixIdentity());
			NorDisEffects::BasicFX->SetMaterial(mSphereMat);
			
			activeSphereTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext);
			m_d3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
		}
	}

	stride = sizeof(Vertex::Basic32);
	offset = 0;

	m_d3dImmediateContext->RSSetState(0);

	m_d3dImmediateContext->IASetInputLayout(NorDisInputLayouts::Basic32);
	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
	m_d3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

	activeSkullTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// Draw the skull
		MATRICES_SET(NorDisEffects::BasicFX, mSkullWorld)

		NorDisEffects::BasicFX->SetMaterial(mSkullMat);

		activeSkullTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext);
		m_d3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);
	}

	mSky->Draw(m_d3dImmediateContext, mCam, NorDisInputLayouts::Pos, NorDisEffects::SkyFX);

	// restore default states, as the SkyFX changes them in the effect file
	m_d3dImmediateContext->RSSetState(0);
	m_d3dImmediateContext->OMSetDepthStencilState(0, 0);

	HR(m_swapChain->Present(0, 0));
	

	return true;
}

#define RENDER_OPTIONS_HELPER(normalMap)\
		switch (mRenderOptions)\
		{\
		case RenderOptions::RenderOptionsNormalMap:\
			dynamic_cast<NormalMapEffect*>(fx)->SetNormalMap(normalMap);\
			break;\
		case RenderOptions::RenderOptionsDisplacementMap:\
			dynamic_cast<DisplacementMapEffect*>(fx)->SetNormalMap(normalMap);\
			dynamic_cast<DisplacementMapEffect*>(fx)->SetViewProj(viewProj);\
			break;\
		}

// The render options in this demo only affect textured objects
// so pick this part out
void NormalDisplacementMapApp::DrawTextured()
{
	XMMATRIX view = mCam.View();
	XMMATRIX proj = mCam.Proj();
	XMMATRIX viewProj = mCam.ViewProj();

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	ID3DX11EffectTechnique* activeTech = NorDisEffects::DisplacementMapFX->Light3TexTech;
	NorDisBasicEffect* fx = 0;

	switch (mRenderOptions)
	{
	case RenderOptionsBasic:
		activeTech = NorDisEffects::BasicFX->Light3TexTech;
		m_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		fx = NorDisEffects::BasicFX;
		break;
	case RenderOptionsNormalMap:
		activeTech = NorDisEffects::NormalMapFX->Light3TexTech;
		m_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		fx = NorDisEffects::NormalMapFX;
		break;
	case RenderOptionsDisplacementMap:
		activeTech = NorDisEffects::DisplacementMapFX->Light3TexTech;
		m_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

		fx = NorDisEffects::DisplacementMapFX;
		break;
	}

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		//
		// Draw the grid
		//

		MATRICES_SET(fx, mGridWorld)
		fx->SetTexTransform(XMMatrixScaling(8.0f, 10.0f, 1.0f));
		fx->SetMaterial(mGridMat);
		fx->SetDiffuseMap(mStoneTexSRV);

		RENDER_OPTIONS_HELPER(mStoneNormalTexSRV)

		activeTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext);
		m_d3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

		//
		// Draw the box
		//

		MATRICES_SET(fx, mBoxWorld)
		fx->SetTexTransform(XMMatrixScaling(2.0f, 1.0f, 1.0f));
		fx->SetMaterial(mBoxMat);
		fx->SetDiffuseMap(mBrickTexSRV);

		RENDER_OPTIONS_HELPER(mBrickNormalTexSRV)

		activeTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext);
		m_d3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

		//
		// Draw the cylinders
		//
		for (int i = 0; i < 10; ++i)
		{
			MATRICES_SET(fx, mCylWorld[i])
			fx->SetTexTransform(XMMatrixScaling(1.0f, 2.0f, 1.0f));
			fx->SetMaterial(mCylinderMat);
			fx->SetDiffuseMap(mBrickTexSRV);

			RENDER_OPTIONS_HELPER(mBrickNormalTexSRV);

			activeTech->GetPassByIndex(p)->Apply(0, m_d3dImmediateContext);
			m_d3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
		}
	}

}


void NormalDisplacementMapApp::BuildShapeGeometryBuffers()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
	geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	mBoxVertexOffset = 0;
	mGridVertexOffset = box.Vertices.size();
	mSphereVertexOffset = mGridVertexOffset + grid.Vertices.size();
	mCylinderVertexOffset = mSphereVertexOffset + sphere.Vertices.size();

	// Cache the index count of each object.
	mBoxIndexCount = box.Indices.size();
	mGridIndexCount = grid.Indices.size();
	mSphereIndexCount = sphere.Indices.size();
	mCylinderIndexCount = cylinder.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	mBoxIndexOffset = 0;
	mGridIndexOffset = mBoxIndexCount;
	mSphereIndexOffset = mGridIndexOffset + mGridIndexCount;
	mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;

	UINT totalVertexCount =
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size();

	UINT totalIndexCount =
		mBoxIndexCount +
		mGridIndexCount +
		mSphereIndexCount +
		mCylinderIndexCount;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Vertex::PosNormalTexTan> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].Tex = box.Vertices[i].TexC;
		vertices[k].TangentU = box.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].Tex = grid.Vertices[i].TexC;
		vertices[k].TangentU = grid.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
		vertices[k].Tex = sphere.Vertices[i].TexC;
		vertices[k].TangentU = sphere.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
		vertices[k].Tex = cylinder.Vertices[i].TexC;
		vertices[k].TangentU = cylinder.Vertices[i].TangentU;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosNormalTexTan) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_d3dDevice->CreateBuffer(&vbd, &vinitData, &mShapesVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(m_d3dDevice->CreateBuffer(&ibd, &iinitData, &mShapesIB));
}