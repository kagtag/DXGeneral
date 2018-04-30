
#include "MathHelper.h"
#include<cmath>

const float MathHelper::Infinity = FLT_MAX;
const float MathHelper::Pi = 3.1415926535f;

float MathHelper::AngleFromXY(float x, float y)
{
	float theta = 0.0f;

	//Quadrant 1 or 4
	if (x >= 0.0f)
	{
		theta = atanf(y / x);

		if (theta < 0.0f)
		{
			theta += 2.0f*Pi;
		}
	}

	else
	{
		theta = atanf(y / x) + Pi;
	}

	return theta;

}

XMVECTOR MathHelper::RandUnitVec3()
{
	XMVECTOR One = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR Zero = XMVectorZero();

	//keep trying until we get a point on/in the hemisphere
	while (true)
	{
		//Generate random point in the cube
		XMVECTOR v = XMVectorSet(RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), 0.0f);

		//Ignore points outside the unit sphere in order to 
		//get an even distribution over the unit sphere,
		if (XMVector3Greater(XMVector3LengthSq(v), One))
			continue;

		return XMVector3Normalize(v);

	}
}

XMVECTOR MathHelper::RandHemisphereUnitVec3(XMVECTOR n)
{
	XMVECTOR One = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR Zero = XMVectorZero();

	while (true)
	{
		XMVECTOR v = XMVectorSet(RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), 0.0f);

		//Ignore points outside the unit sphere in order to 
		//get an even distribution over the unit sphere,
		if (XMVector3Greater(XMVector3LengthSq(v), One))
			continue;

		//Ignore points in the bottom half
		if (XMVector3Less(XMVector3Dot(n, v), Zero))
		{
			continue;
		}
		return XMVector3Normalize(v);

	}
}