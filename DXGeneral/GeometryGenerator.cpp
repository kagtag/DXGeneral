#include"GeometryGenerator.h"

//for HillsDemo
//quad on xz-plane
void GeometryGenerator::CreateGrid(float width, float depth, UINT m, UINT n, MeshData& meshData)
{
	UINT vertexCount = m*n;
	UINT faceCount = (m - 1)*(n - 1) * 2; //number of triangles

	//Create the vertices

	float halfWidth = 0.5f*width;
	float halfDepth = 0.5f*depth;
	
	float dx = width / (n - 1); //with of each cell
	float dz = depth / (m - 1); //depth of each cell

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	meshData.Vertices.resize(vertexCount);
	for (UINT i = 0; i < m; ++i)
	{
		float z = halfDepth - i*dz; //top point
		for (UINT j = 0; j < n; ++j)
		{
			float x = -halfWidth + j*dx; //leftmost point

			//row-major
			meshData.Vertices[i*n + j].Position = XMFLOAT3(x, 0.0f, z);
			meshData.Vertices[i*n + j].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			meshData.Vertices[i*n + j].TangentU = XMFLOAT3(1.0f, 0.0f, 0.0f);

			//Stretch texture over grid
			meshData.Vertices[i*n + j].TexC.x = j*du;
			meshData.Vertices[i*n + j].TexC.y = i*dv;
		}

	}

	//Create the indices
	meshData.Indices.resize(faceCount * 3); //3 indices per face
	
	//Iterate over each quad and compute indices
	UINT k = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (UINT j = 0; j < n - 1; ++j)
		{
			//clockwise
			//topleft triangle
			meshData.Indices[k] = i*n + j;
			meshData.Indices[k + 1] = i*n + j + 1;
			meshData.Indices[k + 2] = (i + 1)*n + j;

			//downright triangle
			meshData.Indices[k + 3] = (i + 1)*n + j;
			meshData.Indices[k + 4] = i*n + j + 1;
			meshData.Indices[k + 5] = (i + 1)*n + j + 1;

			k += 6; //next quad
		}
	}

}