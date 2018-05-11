struct DirectionalLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Direction;
	float pad;
};

struct PointLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	
	float3 Position;
	float Range;
	
	float3 Att;
	float pad;
};

struct SpotLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Direction;
	float Spot;

	float3 Att;
	float pad;
};

struct Material
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular; // w = SpecPower
};

//-------------------------------------------------
//Compute the ambient, diffuse, and specular terms in the lighting equation
//from a directional light, We need to output the terms seperately because
//later we will modify the individual terms.
//-------------------------------------------------
void ComputeDirectionalLight(Material mat, DirectionalLight L,
	float3 normal, float3 toEye,
	out float4 ambient,
	out float4 diffuse,
	out float4 spec)
{

	//Initialize outputs
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//the light vector aims opposite the direction the light rays travel
	float3 lightVec = -L.Direction;
	
	//Add ambient term
	ambient = mat.Ambient*L.Ambient;

	//Add diffuse and specular term, 

	float diffuseFactor = saturate(dot(normal, lightVec));
	
	//Flatten to avoid dynamic branching
	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec = specFactor * mat.Specular * L.Specular;
	}
}


//--------------------------------------------------
//Computes the ambient, diffuse, and specular terms in the lighting equation
//from a point light, We need to output the terms seperately because
//later we will modify the individual terms.
//--------------------------------------------------
void ComputePointLight(Material mat, PointLight L,
	float3 pos, //point light is distance-dependent
	float3 normal, float3 toEye,
	out float4 ambient,
	out float4 diffuse,
	out float4 spec)
{
	//Initialize outputs
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//light vector
	float3 lightVec = L.Position - pos;

	//distance from surface to light
	float d = length(lightVec);



	//Range test
	if (d > L.Range)
		return;

	//Normalize the lightVector
	lightVec /= d;

	//Ambient term
	ambient = mat.Ambient * L.Ambient;

	//Add diffuse the specular term

	float diffuseFactor = dot(lightVec, normal);

	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec = specFactor * mat.Specular * L.Specular;
	}

	//Attenuation
	float att = 1.0f / dot(L.Att, float3(1.0f, d, d*d));

	//Ambient will not be affected
	diffuse *= att;
	spec *= att;
}


//--------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a spotlight. We need to .....
//--------------------------------------------------
void ComputeSpotLight(Material mat, SpotLight L,
	float3 pos,
	float3 normal, float3 toEye,
	out float4 ambient,
	out float4 diffuse,
	out float4 spec)
{
	//Initialize outputs
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//light vector
	float3 lightVec = L.Position - pos;

	//distance
	float d = length(lightVec);

	//Range test
	if (d > L.Range)
		return;

	//Normalize the light vector
	lightVec /= d;

	//Ambient term
	ambient = mat.Ambient * L.Ambient;

	//diffuse and specular term
	float diffuseFactor = dot(lightVec, normal);

	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0), mat.Specular.w);

		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec = specFactor * mat.Specular * L.Specular;
	}

	//Scale by spotlight factor and attenuation
	float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);

	float att = spot / dot(L.Att, float3(1.0f, d, d*d));

	ambient *= spot; //ambient will not affected by attenuation
	diffuse *= att;
	spec *= att;
}