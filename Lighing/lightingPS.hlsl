#include "lighting.hlsli"

cbuffer cbPerFrame
{
	DirectionalLight gDirLight;
	PointLight gPointLight;
	SpotLight gSpotLight;
	Material gMaterial;
};

cbuffer EyeBuffer
{
	float3 gEyePosW;
	float pad;
};

struct PixelIn
{
	float4 PosH : SV_POSITION;
	float4 PosW : POSITION;
	float3 NormalW : NORMAL;
};

float4 LightingPixelShader(PixelIn pin):SV_TARGET
{
	//Interpolating normal can unnormal it, so normalize it
	pin.NormalW = normalize(pin.NormalW);

	float3 toEyeW = normalize(gEyePosW - pin.PosW.xyz);

	//Start with a sum of zero
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//Sum the light contribution from each light source
	float4 A, D, S;

	ComputeDirectionalLight(gMaterial, gDirLight, pin.NormalW, toEyeW, A, D, S);
	ambient += A;
	diffuse += D;
	spec += S;

	ComputePointLight(gMaterial, gPointLight, pin.PosW.xyz, pin.NormalW, toEyeW, A, D, S);
	ambient += A;
	diffuse += D;
	spec += S;

	ComputeSpotLight(gMaterial, gSpotLight, pin.PosW.xyz, pin.NormalW, toEyeW, A, D, S);
	ambient += A;
	diffuse += D;
	spec += S;

	float4 litColor = ambient + diffuse + spec;

	//Common to take alpha from diffuse material
	litColor.a = gMaterial.Diffuse.a;

	return litColor;

}