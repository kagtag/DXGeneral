
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;

	matrix worldInvTrans;
};



//////////////////////////////
////TYPEDEFS
/////////////////////////////
struct VertexIn
{
	float4 PosL  : POSITION;
	float4 NormalL : NORMAL;
};

struct PixelIn
{
	float4 PosH : SV_POSITION;
	float4 PosW : POSITION;
	float3 NormalW : NORMAL;
};

PixelIn LightingVertexShader(VertexIn vin)
{
	PixelIn vout;

	vin.PosL.w = 1.0f;

	//Compute position in world space
	//for computing eye vector later
	vout.PosW = mul(vin.PosL, worldMatrix);


	//Transform Positon
	vout.PosH = mul(vin.PosL, worldMatrix);
	vout.PosH = mul(vout.PosH, viewMatrix);
	vout.PosH = mul(vout.PosH, projectionMatrix);

	//Transform Normal
	vout.NormalW = mul(vin.NormalL, (float3x3)worldInvTrans);

	return vout;
}
