cbuffer UniformBuffer : register(b0) {
	float4x4 projectionMatrix;
};

struct VertexInput {
	float2 position : POSITION0;
	float4 color : COLOR0;
};

struct PixelInput {
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};

PixelInput main(VertexInput input) {
	PixelInput output;

	output.position = mul(projectionMatrix, float4(input.position, 0.0, 1.0));
	output.color = input.color;
	
	return output;
}