
cbuffer cbPerFrame
{
	float3 gEyePosW;

	// for when the emit position/direction is varying
	float3 gEmitPosW;
	float3 gEmitDirW;

	float gGameTime;
	float gTimeStep;
	float4x4 gViewProj;

};

cbuffer cbFixed
{
	// Net constant acceleration used to accelerate the particles
	float3 gAccelW = { 0.0f, 7.8f, 0.0f };

	// Textured coordinates used to stretch texture over quad 
	// when we expand point particle into a quad
	float2 gQuadTexC[4] =
	{
		float2(0.0f,1.0f),
		float2(1.0f,1.0f),
		float2(0.0f,0.0f),
		float2(1.0f,0.0f)
	};
};

// Array of textures for texturing the particles
Texture2DArray gTexArray;

// Random texture used to generate random numbers in shaders
Texture1D gRandomTex;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

DepthStencilState DisableDepth
{
	DepthEnable = false;
	DepthWriteMask = ZERO;
};

DepthStencilState NoDepthWrites
{
	DepthEnable = true;
	DepthWriteMask = ZERO;
};

BlendState AdditiveBlending
{
	AlphaToCoverageEnable = false;
	BlendEnable[0] = true;
	
	SrcBlend = SRC_ALPHA;
	DestBlend = ONE;
	BlendOp = ADD;

	SrcBlendAlpha = ZERO;
	DestBlendAlpha = ZERO;
	BlendOpAlpha = ADD;
	RenderTargetWriteMask[0] = 0x0F;
};

//
// Helper functions
//
float3 RandUnitVec3(float offset)
{
	// Use game time plus offset to sample random texture
	float u = (gGameTime + offset);

	// coordinates in [-1,1]
	float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;

	// project onto unit sphere
	return normalize(v);
}


///////////////////
// STREAM-OUT TECH
///////////////////

#define PT_EMITTER 0
#define PT_FLARE 1

struct Particle
{
	float3 InitialPosW : POSITION;
	float3 InitialVelW : VELOCITY;
	float2 SizeW : SIZE;
	float Age : AGE;
	uint Type : TYPE;
};

Particle StreamOutVS(Particle vin)
{
	return vin;
}

// The stream-out GS is just responsible for emitting
// new particles and destroying old particles. The logic 
// programmed here will generally vary from particle system
// to particle system, as the destroy/spam rules will be different.
[maxvertexcount(2)]
void StreamOutGS(point Particle gin[1],
	inout PointStream<Particle> ptStream)
{
	gin[0].Age += gTimeStep;

	if (gin[0].Type == PT_EMITTER)
	{
		// time to emit a new particle ?
		if (gin[0].Age > 0.005f)
		{
			float3 vRandom = RandUnitVec3(0.0f);
			vRandom.x *= 0.5f;
			vRandom.y *= 0.5f;

			Particle p;
			p.InitialPosW = gEmitPosW.xyz;
			p.InitialVelW = 4.0f *vRandom;
			p.SizeW = float2(3.0f, 3.0f);
			p.Age = 0.0f;
			p.Type = PT_FLARE;

			ptStream.Append(p);

			// reset the time to emit
			gin[0].Age = 0.0f;
		}

		// Always keep emitters
		ptStream.Append(gin[0]);
	}
	else
	{
		// Specify condition to keep particle, this may vary from system to system
		if (gin[0].Age <= 1.0f)
			ptStream.Append(gin[0]);
	}
}

// using stream out, must create Geometry Shader specially
GeometryShader gsStreamOut = ConstructGSWithSO(
	CompileShader(gs_5_0, StreamOutGS()),
	"POSITION.xyz; VELOCITY.xyz; SIZE.xy; AGE.x; TYPE.x"
);

technique11 StreamOutTech
{
	pass p0
	{
		SetVertexShader(CompileShader(vs_5_0, StreamOutVS()));
		SetGeometryShader(gsStreamOut);

		// disable pixel shader for stream out only
		SetPixelShader(NULL);

		// we must also disable the depth buffer for stream-out only
		SetDepthStencilState(DisableDepth, 0);
	}
}

//////////////
// DRAW TECH
/////////////

struct VertexOut
{
	float3 PosW : POSITION;
	float2 SizeW : SIZE;
	float4 Color : COLOR;
	uint Type : TYPE;
};

VertexOut DrawVS(Particle vin)
{
	VertexOut vout;

	float t = vin.Age;

	// constant accleration euqation
	vout.PosW = 0.5f*t*t*gAccelW + t*vin.InitialVelW + vin.InitialPosW;

	// fade color with time
	float opacity = 1.0f - smoothstep(0.0f, 1.0f, t / 1.0f);
	vout.Color = float4(1.0f, 1.0f, 1.0f, opacity);

	vout.SizeW = vin.SizeW;
	vout.Type = vin.Type;

	return vout;

}

struct GeoOut
{
	float4 PosH : SV_Position;
	float4 Color : COLOR;
	float2 Tex : TEXCOORD;
};

// The draw GS just expands points into camera facing quads
[maxvertexcount(4)]
void DrawGS(point VertexOut gin[1],
	inout TriangleStream<GeoOut> triStream)
{
	// do not draw emitter particles
	if (gin[0].Type != PT_EMITTER)
	{
		//
		// Compute world matrix so that billboard faces the camera
		//
		float3 look = normalize(gEyePosW.xyz - gin[0].PosW);
		float3 right = normalize(cross(float3(0, 1, 0), look));
		float3 up = cross(look, right);

		//
		// compute triangle strip vertices (quad) in world space
		//
		float halfWidth = 0.5f*gin[0].SizeW.x;
		float halfHeight = 0.5f*gin[0].SizeW.y;

		float4 v[4];
		v[0] = float4(gin[0].PosW + halfWidth*right - halfHeight* up, 1.0f);
		v[1] = float4(gin[0].PosW + halfWidth*right + halfHeight* up, 1.0f);
		v[2] = float4(gin[0].PosW - halfWidth*right - halfHeight* up, 1.0f);
		v[3] = float4(gin[0].PosW - halfWidth*right + halfHeight* up, 1.0f);

		//
		// Transform quad vertices to world space and output
		// them as a triangle strip
		GeoOut gout;
		for (int i = 0; i < 4; ++i)
		{
			gout.PosH = mul(v[i], gViewProj);
			gout.Tex = gQuadTexC[i];
			gout.Color = gin[0].Color;
			triStream.Append(gout);
		}
	}
}

float4 DrawPS(GeoOut pin) : SV_TARGET
{
	return gTexArray.Sample(samLinear, float3(pin.Tex, 0))*pin.Color;
}

technique11 DrawTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, DrawVS()));
		SetGeometryShader(CompileShader(gs_5_0, DrawGS()));
		SetPixelShader(CompileShader(ps_5_0, DrawPS()));

		SetBlendState(AdditiveBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);
		SetDepthStencilState(NoDepthWrites, 0);
	}
}