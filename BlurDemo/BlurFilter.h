#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include "d3dUtil.h"

class BlurFilter
{
public:
	BlurFilter();
	~BlurFilter();

	ID3D11ShaderResourceView* GetBlurredOutput();

	// Generate Gaussian blur weights
	void SetGaussianWeights(float sigma);

	// Manually specify blur weights
	void SetWeights(const float weights[9]);

	// The width and height should match the dimension of the input texture
	// to blur
	void Init(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format);

	// Blur the input texture blurCount times, note that this modifies the input texture, not a copy of it
	void BlurInPlace(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* inputSRV, ID3D11UnorderedAccessView* inputUAV, int blurCount);

private:
	UINT mWidth;
	UINT mHeight;
	DXGI_FORMAT mFormat;

	ID3D11ShaderResourceView* mBlurredOutputTexSRV;
	ID3D11UnorderedAccessView* mBlurredOutputTexUAV;
};