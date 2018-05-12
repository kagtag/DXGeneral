#pragma once

#include<Windows.h>
#include<DirectXMath.h>

using namespace DirectX;

class Waves
{
public:
	Waves();
	~Waves();

	UINT RowCount()const;
	UINT ColumnCount()const;
	UINT VertexCount()const;
	UINT TriangleCount()const;
	float Width()const;
	float Depth()const;

	//Returns the solution at the ith grid point
	const XMFLOAT3& operator[](int i)const { return m_currSolution[i]; }

	const XMFLOAT3& Normal(int i)const { return m_normals[i]; }
	const XMFLOAT3& TangentX(int i)const { return m_tangentX[i]; }

	void Init(UINT m, UINT n, float dx, float dt, float speed, float damping);
	void Update(float dt);
	void Disturb(UINT i, UINT j, float magnitude);

private:
	UINT m_numRows;
	UINT m_numCols;

	UINT m_vertexCount;
	UINT m_triangleCount;

	//Simulation constants we can precompute
	float m_k1;
	float m_k2;
	float m_k3;

	float m_timeStep;
	float m_spatialStep;

	XMFLOAT3* m_prevSolution;
	XMFLOAT3* m_currSolution;

	XMFLOAT3* m_normals;
	XMFLOAT3* m_tangentX;

};