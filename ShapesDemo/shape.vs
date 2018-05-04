

cbuffer cbPerObject
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

//////////////////////////////
////TYPEDEFS
/////////////////////////////
struct VertexIn
{
	float4 PosL  : POSITION;
	float4 Color : COLOR;
};

struct PixelIn
{
	float4 PosH : SV_POSITION;
	float4 Color : COLOR;
};

PixelIn ShapeVertexShader(VertexIn vin)
{
	PixelIn vout;
	
	vin.PosL.w=1.0f;
	
	vout.PosH = mul(vin.PosL,worldMatrix);
	vout.PosH = mul(vout.PosH,viewMatrix);
	vout.PosH = mul(vout.PosH,projectionMatrix);
	
	//Store the input color for the pixel shader to use
	vout.Color = vin.Color;
	
	return vout;
}