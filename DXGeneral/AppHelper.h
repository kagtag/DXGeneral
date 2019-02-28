
#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"

#include "Camera.h"

#include "RenderStates.h"

#if defined(DEBUG) | defined(_DEBUG) 

#define DEFAULT_WINMAIN(className) \
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, \
	PSTR cmdLine, int showCmd) \
{ \
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); \
	className theApp(hInstance); \
	if (!theApp.Init()) \
		return 0; \
	return theApp.Run(); \
}

#else

#define DEFAULT_WINMAIN(className) \
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, \
	PSTR cmdLine, int showCmd) \
{ \
	className theApp(hInstance); \
	if (!theApp.Init()) \
		return 0; \
	return theApp.Run(); \
}

#endif 

#define MATRICES_SET(effect, worldMat)\
world = XMLoadFloat4x4(&worldMat);\
worldInvTranspose = MathHelper::InverseTranspose(world);\
worldViewProj = world*view*proj;\
effect->SetWorld(world);\
effect->SetWorldInvTranspose(worldInvTranspose);\
effect->SetWorldViewProj(worldViewProj);


// Common methods appear in almost all Apps
// mainly First Person Camera operation
class CommonApp : public D3DApp
{
public:
	CommonApp(HINSTANCE hInstance);

	void OnResize();

	void UpdateScene(float dt);

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

protected:

	Camera mCam;

	POINT mLastMousePos;
};


// Shapes scene in the book
// Geometries, textures, materials and lights
class ShapesBaseApp : public CommonApp
{
public:
	ShapesBaseApp(HINSTANCE hInstance);
	~ShapesBaseApp();

	bool Init();

protected:
	virtual void BuildShapeGeometryBuffers();
	void BuildSkullGeometryBuffers();

protected:

	ID3D11Buffer* mShapesVB;
	ID3D11Buffer* mShapesIB;

	ID3D11Buffer* mSkullVB;
	ID3D11Buffer* mSkullIB;

	ID3D11ShaderResourceView* mFloorTexSRV;
	ID3D11ShaderResourceView* mStoneTexSRV;
	ID3D11ShaderResourceView* mBrickTexSRV;

	DirectionalLight mDirLights[3];
	Material mGridMat;
	Material mBoxMat;
	Material mCylinderMat;
	Material mSphereMat;

	Material mSkullMat;

	//Material mCenterSphereMat;

	// Define transformations from local spaces to world space.
	XMFLOAT4X4 mSphereWorld[10];
	XMFLOAT4X4 mCylWorld[10];
	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mGridWorld;

	XMFLOAT4X4 mSkullWorld; // just load the skull model, but do nothing to its position

	int mBoxVertexOffset;
	int mGridVertexOffset;
	int mSphereVertexOffset;
	int mCylinderVertexOffset;

	UINT mBoxIndexOffset;
	UINT mGridIndexOffset;
	UINT mSphereIndexOffset;
	UINT mCylinderIndexOffset;

	UINT mBoxIndexCount;
	UINT mGridIndexCount;
	UINT mSphereIndexCount;
	UINT mCylinderIndexCount;

	UINT mSkullIndexCount;

	UINT mLightCount;

};
