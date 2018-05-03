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


void GeometryGenerator::CreateCylinder(float bottomRadius, float topRadius, float height,
	UINT sliceCount, UINT stackCount, MeshData& meshData)
{
	meshData.Vertices.clear();
	meshData.Indices.clear();

	//Build stacks

	float stackHeight = height / stackCount;

	//Amount to increment radius as we move up each stack level from bottom to top
	float radiusStep = (topRadius - bottomRadius) / stackCount;

	UINT ringCount = stackCount + 1; // n stack, n+1 rings

	//Compute vertices for each stack ring starting at the bottom and moving up
	for (UINT i = 0; i < ringCount; ++i)
	{
		float y = -0.5f*height + i*stackHeight; //height of the ring
		float r = bottomRadius + i*radiusStep; //radius of the ring

		//vertices of ring 
		float dTheta = 2.0f*XM_PI / sliceCount; // central angle for each fan
		for (UINT j = 0; j <= sliceCount; ++j) // n slice, n+1 vertices per ring
		{
			Vertex vertex;

			float c = cosf(j*dTheta);
			float s = sinf(j*dTheta);

			vertex.Position = XMFLOAT3(r*c, y, r*s); // coordinate for each vertex

			vertex.TexC.x = (float)j / sliceCount; //increasing along the ring
			vertex.TexC.y = 1.0f - (float)i / stackCount; //decreasing along the stack 

			//Features for texturing and lighting leave them for latter chapter

			//Cylinder can be parameterized as follows, where we introduce v
			//parameter that goes in...

			vertex.TangentU = XMFLOAT3(-s, 0.0f, c);

			float dr = bottomRadius - topRadius;
			XMFLOAT3 bitangent(dr*c, -height, dr*s);

			XMVECTOR T = XMLoadFloat3(&vertex.TangentU);
			XMVECTOR B = XMLoadFloat3(&bitangent);
			XMVECTOR N = XMVector3Normalize(XMVector3Cross(T, B));
			XMStoreFloat3(&vertex.Normal, N);

			meshData.Vertices.push_back(vertex);

		}
	}

	//Add one because we duplicate the first and last vertex per ring
	//since the texture coordinates are different
	UINT ringVertexCount = sliceCount + 1;

	//Compute indices for each stack.
	for (UINT i = 0; i < stackCount; ++i)
	{
		for (UINT j = 0; i < sliceCount; ++j)
		{
			//Clockwise
			//topleft triangle
			meshData.Indices.push_back(i*ringVertexCount + j);
			meshData.Indices.push_back((i + 1)*ringVertexCount + j);
			meshData.Indices.push_back((i + 1)*ringVertexCount + j + 1);

			//downright triangle
			meshData.Indices.push_back(i*ringVertexCount + j);
			meshData.Indices.push_back((i + 1)*ringVertexCount + j + 1);
			meshData.Indices.push_back(i*ringVertexCount + j + 1);

		}
	}

	BuildCylinderTopCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);
	BuildCylinderBottomCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);

}

void GeometryGenerator::BuildCylinderTopCap(float bottomRadius, float topRadius, float height,
	UINT sliceCount, UINT stackCount, MeshData& meshData)
{
	UINT baseIndex = (UINT)meshData.Vertices.size();

	float y = 0.5f*height;
	float dTheta = 2.0f*XM_PI / sliceCount;

	//Duplicate cap ring vertices because the texture coordinates and normals differ
	for (UINT i = 0; i <= sliceCount; ++i)
	{
		float x = topRadius*cosf(i*dTheta);
		float z = topRadius*sinf(i*dTheta);

		//Scale down by the height to try and make top cap texture coord 
		//area proportional to base
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.Vertices.push_back(Vertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	//Cap center vertex
	meshData.Vertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	//Index of center vertex
	UINT centerIndex = (UINT)meshData.Vertices.size() - 1; //last vertex

	for (UINT i = 0; i < sliceCount; ++i)
	{
		meshData.Indices.push_back(centerIndex);
		meshData.Indices.push_back(baseIndex + i + 1);
		meshData.Indices.push_back(baseIndex + i);
	}
}

void GeometryGenerator::BuildCylinderBottomCap(float bottomRadius, float topRadius, float height,
	UINT sliceCount, UINT stackCount, MeshData& meshData)
{
	UINT baseIndex = (UINT)meshData.Vertices.size();

	float y = -0.5f*height;
	float dTheta = 2.0f*XM_PI / sliceCount;

	//Duplicate cap ring vertices because the texture coordinates and normals differ
	for (UINT i = 0; i <= sliceCount; ++i)
	{
		float x = bottomRadius*cosf(i*dTheta);
		float z = bottomRadius*sinf(i*dTheta);

		//Scale down by the height to try and make top cap texture coord 
		//area proportional to base
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.Vertices.push_back(Vertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	//Cap center vertex
	meshData.Vertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	//Index of center vertex
	UINT centerIndex = (UINT)meshData.Vertices.size() - 1; //last vertex

	for (UINT i = 0; i < sliceCount; ++i)
	{
		meshData.Indices.push_back(centerIndex);
		meshData.Indices.push_back(baseIndex + i + 1);
		meshData.Indices.push_back(baseIndex + i);
	}
}

void GeometryGenerator::CreateSphere(float radius, UINT sliceCount, UINT stackCount, MeshData& meshData)
{
	meshData.Vertices.clear();
	meshData.Indices.clear();

	//Poles: note that there will be texture coordinate distortion as there
	//is not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere
	Vertex topVertex(0.0f, +radius, 0.0f, //Position
		0.0f, +1.0f, 0.0f, //Normal
		1.0f, 0.0f, 0.0f, //TangentU
		0.0f, 0.0f); //TexC
	Vertex bottomVertex(0.0f, -radius, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f);

	meshData.Vertices.push_back(topVertex);

	float phiStep = XM_PI / stackCount;  //vertical
	float thetaStep = 2.0f*XM_PI / sliceCount; //horizontal

	//Compute vertices for each stack ring ( do not count the poles as rings
	for (UINT i = 1; i <= stackCount - 1; ++i)
	{
		float phi = i*phiStep;  //from top to bottom

		//Vertices of ring
		for (UINT j = 0; j <= sliceCount; ++j)
		{
			float theta = j*thetaStep;

			Vertex v;

			//spherical to cartesian
			v.Position.x = radius*sinf(phi)*cosf(theta);
			v.Position.z = radius*sinf(phi)*sinf(theta);
			v.Position.y = radius*cosf(phi);

			//Partial derivative of P with respect to theta
			v.TangentU.x = -radius*sinf(phi)*sinf(theta);
			v.TangentU.z = +radius*sinf(phi)*cosf(theta);
			v.TangentU.y = 0.0f;

			XMVECTOR T = XMLoadFloat3(&v.TangentU);
			XMStoreFloat3(&v.TangentU, XMVector3Normalize(T));

			XMVECTOR p = XMLoadFloat3(&v.Position);
			XMStoreFloat3(&v.Normal, XMVector3Normalize(p));

			v.TexC.x = theta / XM_2PI;
			v.TexC.y = phi / XM_PI;

			meshData.Vertices.push_back(v);
		}
	}

	meshData.Vertices.push_back(bottomVertex);

	//Compute indices for top stack. The top stack was written first to the vertex buffer
	//and connects the top pole to the first ring.
	for (UINT i = 1; i <= sliceCount; ++i)
	{
		meshData.Indices.push_back(0);
		meshData.Indices.push_back(i + 1);
		meshData.Indices.push_back(i);
	}

	//Compute indices for inner stacks(not connected to poles

	//Offset the indices to the index of the first vertex in the first ring
	//This is just skipping the top pole vertex
	UINT baseIndex = 1;
	UINT ringVertexCount = sliceCount + 1;
	for (UINT i = 0; i < sliceCount - 2; ++i) //the top and bottom stacks are special
	{
		for (UINT j = 0; j < sliceCount; ++j) //number of quads per ring = number of slices
		{
			//topleft triangle
			meshData.Indices.push_back(baseIndex + i*ringVertexCount + j);
			meshData.Indices.push_back(baseIndex + i*ringVertexCount + j + 1);
			meshData.Indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);

			//downright triangle
			meshData.Indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);
			meshData.Indices.push_back(baseIndex + i*ringVertexCount + j + 1);
			meshData.Indices.push_back(baseIndex + (i + 1)*ringVertexCount + j + 1);

		}
	}

	//Compute indices for bottom stack, the bottom stack was written last to the vertex buffer
	//and connects the bottom pole to the bottom ring

	UINT southPoleIndex = (UINT)meshData.Vertices.size() - 1;

	//Offset the indices to the index of the first vertex in the last ring
	baseIndex = southPoleIndex - ringVertexCount;
	for (UINT i = 0; i < sliceCount; ++i)
	{
		meshData.Indices.push_back(southPoleIndex);
		meshData.Indices.push_back(baseIndex + i);
		meshData.Indices.push_back(baseIndex + i + 1);
	}
}


void GeometryGenerator::CreateGeosphere(float radius, UINT numSubdivisions, MeshData& meshData)
{
	//Pus a cap on the number of subdivisions
	numSubdivisions = MathHelper::Min(numSubdivisions, 5u);

	//Approximate a sphere by tessellating an icosahedron (20 faces)
	const float X = 0.525731f;
	const float Z = 0.850651f;

	XMFLOAT3 pos[12] =
	{
		XMFLOAT3(-X, 0.0f, Z),  XMFLOAT3(X, 0.0f, Z),
		XMFLOAT3(-X, 0.0f, -Z), XMFLOAT3(X, 0.0f, -Z),
		XMFLOAT3(0.0f, Z, X),   XMFLOAT3(0.0f, Z, -X),
		XMFLOAT3(0.0f, -Z, X),  XMFLOAT3(0.0f, -Z, -X),
		XMFLOAT3(Z, X, 0.0f),   XMFLOAT3(-Z, X, 0.0f),
		XMFLOAT3(Z, -X, 0.0f),  XMFLOAT3(-Z, -X, 0.0f)
	};

	DWORD k[60] =
	{
		1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
		1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
		10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
	};

	meshData.Vertices.resize(12);
	meshData.Indices.resize(60);

	for (UINT i = 0; i < 12; ++i)
	{
		meshData.Vertices[i].Position = pos[i];
	}

	for (UINT i = 0; i < 60; ++i)
	{
		meshData.Indices[i] = k[i];
	}

	for (UINT i = 0; i < numSubdivisions; ++i)
	{
		Subdivide(meshData);
	}

	//Project vertices onto sphere and scale.
	for (UINT i = 0; i < meshData.Vertices.size(); ++i)
	{
		//Project onto unit sphere
		XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&meshData.Vertices[i].Position));

		//Project onto sphere
		XMVECTOR p = radius*n;

		XMStoreFloat3(&meshData.Vertices[i].Position, p);
		XMStoreFloat3(&meshData.Vertices[i].Normal, n);

		//Derive texture coordinates from spherical coordinates
		float theta = MathHelper::AngleFromXY(
			meshData.Vertices[i].Position.x,
			meshData.Vertices[i].Position.y
		);

		float phi = acosf(meshData.Vertices[i].Position.y / radius);
		
		meshData.Vertices[i].TexC.x = theta / XM_2PI;
		meshData.Vertices[i].TexC.y = phi / XM_PI;

		//Partial derivative of P with respect to theta
		meshData.Vertices[i].TangentU.x = -radius*sinf(phi)*sinf(theta);
		meshData.Vertices[i].TangentU.z = +radius*sinf(phi)*sinf(theta);
		meshData.Vertices[i].TangentU.y = 0.0f;

		//Normalize tangentU
		XMVECTOR T = XMLoadFloat3(&meshData.Vertices[i].TangentU);
		XMStoreFloat3(&meshData.Vertices[i].TangentU, XMVector3Normalize(T));

	}

}

void GeometryGenerator::Subdivide(MeshData& meshData)
{
	//Save a copy of the input geometry
	MeshData inputCopy = meshData;

	meshData.Vertices.resize(0);
	meshData.Indices.resize(0);

	//       v1
	//       *
	//      / \
	//     /   \
	//  m0*-----*m1
	//   / \   / \
	//  /   \ /   \
	// *-----*-----*
	// v0    m2     v2

	UINT numTris = inputCopy.Indices.size() / 3;

	for (UINT i = 0; i < numTris; ++i)
	{
		Vertex v0 = inputCopy.Vertices[inputCopy.Indices[i * 3 + 0]];
		Vertex v1 = inputCopy.Vertices[inputCopy.Indices[i * 3 + 1]];
		Vertex v2 = inputCopy.Vertices[inputCopy.Indices[i * 3 + 2]];

		//Generate the midpoints

		Vertex m0, m1, m2;

		//For subdivision, we just care about the position component,
		//We derive the other vertex components in CreateGeosphere
		m0.Position = XMFLOAT3(
			0.5f*(v0.Position.x + v1.Position.x),
			0.5f*(v0.Position.y + v1.Position.y),
			0.5f*(v0.Position.z + v1.Position.z));

		m1.Position = XMFLOAT3(
			0.5f*(v1.Position.x + v2.Position.x),
			0.5f*(v1.Position.y + v2.Position.y),
			0.5f*(v1.Position.z + v2.Position.z));

		m2.Position = XMFLOAT3(
			0.5f*(v0.Position.x + v2.Position.x),
			0.5f*(v0.Position.y + v2.Position.y),
			0.5f*(v0.Position.z + v2.Position.z));

		//Add new Geometry

		meshData.Vertices.push_back(v0);
		meshData.Vertices.push_back(v1);
		meshData.Vertices.push_back(v2);

		meshData.Vertices.push_back(m0);
		meshData.Vertices.push_back(m1);
		meshData.Vertices.push_back(m2);

		meshData.Indices.push_back(i * 6 + 0);
		meshData.Indices.push_back(i * 6 + 3);
		meshData.Indices.push_back(i * 6 + 5);

		meshData.Indices.push_back(i * 6 + 3);
		meshData.Indices.push_back(i * 6 + 4);
		meshData.Indices.push_back(i * 6 + 5);

		meshData.Indices.push_back(i * 6 + 5);
		meshData.Indices.push_back(i * 6 + 4);
		meshData.Indices.push_back(i * 6 + 2);

		meshData.Indices.push_back(i * 6 + 3);
		meshData.Indices.push_back(i * 6 + 1);
		meshData.Indices.push_back(i * 6 + 4);
	}

}

