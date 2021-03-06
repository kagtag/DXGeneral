#include"Waves.h"
#include<algorithm>
#include<vector>
#include<cassert>

Waves::Waves()
	:m_numRows(0), m_numCols(0), m_vertexCount(0), m_triangleCount(0),
	m_k1(0.0f), m_k2(0.0f), m_k3(0.0f), m_timeStep(0.0f), m_spatialStep(0.0f),
	m_prevSolution(0), m_currSolution(0)
{

}

Waves::~Waves()
{
	delete[] m_prevSolution;
	delete[] m_currSolution;
}

UINT Waves::RowCount()const
{
	return m_numRows;
}

UINT Waves::ColumnCount()const
{
	return m_numCols;
}

UINT Waves::VertexCount()const
{
	return m_vertexCount;
}

UINT Waves::TriangleCount()const
{
	return m_triangleCount;
}

//
//Approximation of differential equations
//dx: distance between adjacent vertices, in both x and z direction
//dt: timestep between each iteration
//speed: propagation speed of tension
//damping: viscous damping factor to ensure the motion stops in finite time
//
void Waves::Init(UINT m, UINT n, float dx, float dt, float speed, float damping)
{
	m_numRows = m;
	m_numCols = n;

	m_vertexCount = m*n;
	m_triangleCount = (m - 1)*(n - 1) * 2;

	m_timeStep = dt;
	m_spatialStep = dx;

	//Refer to equation 15.25
	float d = damping*dt + 2.0f;
	float e = (speed*speed)*(dt*dt) / (dx*dx);
	m_k1 = (damping*dt - 2.0f) / d;
	m_k2 = (4.0f - 8.0f*e) / d;
	m_k3 = (2.0f*e) / d;

	//In case Init() called agin.
	delete[] m_prevSolution;
	delete[] m_currSolution;

	delete[] m_normals;
	delete[] m_tangentX;

	m_prevSolution = new XMFLOAT3[m*n];
	m_currSolution = new XMFLOAT3[m*n];

	m_normals = new XMFLOAT3[m*n];
	m_tangentX = new XMFLOAT3[m*n];

	//Generate grid vertices in system memory
	float halfWidth = (n - 1)*dx*0.5f;
	float halfDepth = (m - 1)*dx*0.5f;
	for (UINT i = 0; i < m; ++i)
	{
		float z = halfDepth - i*dx;
		for (UINT j = 0; j < n; ++j)
		{
			float x = -halfWidth + j*dx;

			//row major
			m_prevSolution[i*n + j] = XMFLOAT3(x, 0.0f, z);
			m_currSolution[i*n + j] = XMFLOAT3(x, 0.0f, z);

			m_normals[i*n + j] = XMFLOAT3(0.0f, 1.0f, 0.0f);
			m_tangentX[i*n + j] = XMFLOAT3(1.0f, 0.0f, 0.0f);
		}
	}

}


//if the propagation speed is too fast or the timeStep too long,
//the iteration diverges to infinity.
//Stable conditions exist
void Waves::Update(float dt)
{
	static float t = 0;
	
	//Accumulate time
	t += dt;

	//Only update the simulation at the specified time step
	if (t > m_timeStep)
	{
		//Only update interior points; we use zero boundary conditions
		for (DWORD i = 1; i < m_numRows - 1; ++i)
		{
			for (DWORD j = 1; j < m_numCols - 1; ++j)
			{
				//After this update, we will be discarding the old previous
				//buffer, so overwrite that buffer with the new update.
				//Note how we can do this inplace(read/write to same element)
				//because we won't need prev_ij again and the assignment happens last
				
				//Note j indexes x and i indexes z; h(x_j, z_i, t_k)
				//Moreover, our +z axis goes "down", this is just to 
				//keep consistent with our row indices going down

				//Note that the current previous buffer become next frame's current buffer, and vice versa
				m_prevSolution[i*m_numCols + j].y =
					m_k1*m_prevSolution[i*m_numCols + j].y +
					m_k2*m_currSolution[i*m_numCols + j].y +

					m_k3*(m_currSolution[(i + 1) *m_numCols + j].y +
						m_currSolution[(i - 1)*m_numCols + j].y +
						m_currSolution[i*m_numCols + j + 1].y +
						m_currSolution[i*m_numCols + j - 1].y);
				
			}
		}

		//We just overwrote the previous buffer with the new data, so
		//this data needs to become the current solution and the old
		//current solution becomes the new previous solution
		std::swap(m_prevSolution, m_currSolution);

		t = 0.0f; //reset time

		//Compute normals using finite difference scheme
		for (UINT i = 1; i < m_numRows-1; ++i)
		{
			for (UINT j = 1; j < m_numCols-1; ++j)
			{
				//4 points surrounding the ijth vertex 
				float l = m_currSolution[i*m_numCols + j - 1].y;
				float r = m_currSolution[i*m_numCols + j + 1].y;
				float t = m_currSolution[(i - 1)*m_numCols + j].y;
				float b = m_currSolution[(i + 1)*m_numCols + j].y;

				m_normals[i*m_numCols + j].x = -r + l;
				m_normals[i*m_numCols + j].y = 2.0f*m_spatialStep;
				m_normals[i*m_numCols + j].z = b - t;

				//normalization
				XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&m_normals[i*m_numCols + j]));
				XMStoreFloat3(&m_normals[i*m_numCols + j], n);

				//
				m_tangentX[i*m_numCols + j] = XMFLOAT3(2.0f*m_spatialStep, r - l, 0.0f);
				XMVECTOR T = XMVector3Normalize(XMLoadFloat3(&m_tangentX[i*m_numCols + j]));
				XMStoreFloat3(&m_tangentX[i*m_numCols + j], T);

			}
		}
	}
}

void Waves::Disturb(UINT i, UINT j, float magnitude)
{
	//Don't disturb boundaries
	assert(i > 1 && j < m_numRows - 2);
	assert(j > 1 && j < m_numCols - 2);

	float halfMag = 0.5f*magnitude;

	//disturb the ijth vertex height and its neighbors
	m_currSolution[i*m_numCols + j].y += magnitude;

	m_currSolution[i*m_numCols + j + 1].y += halfMag;
	m_currSolution[i*m_numCols + j - 1].y += halfMag;
	m_currSolution[(i + 1)*m_numCols + j].y += halfMag;
	m_currSolution[(i - 1)*m_numCols + j].y += halfMag;

}