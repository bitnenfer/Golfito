Texture2D colorTexture;
SamplerState textureSampler;

struct PixelInput {
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
	float4 color : COLOR0;
};

float4 main(PixelInput input) : SV_TARGET{
	return colorTexture.Sample(textureSampler, input.texCoord) * input.color.abgr;
}