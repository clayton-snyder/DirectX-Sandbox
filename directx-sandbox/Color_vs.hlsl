// Globals - will be modified externally
cbuffer MatrixBuffer {
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

/* ": POSITION", ": COLOR", etc. are semantics for the GPU to know how to use the variable. */

// Input type for vertex shader; also matches vertex buffer type
struct VertexInput {
	float4 position : POSITION;
	float4 color : COLOR;
};

// Output of vertex shader and is sent to pixel shader
struct PixelInput {
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PixelInput ColorVertexShader(VertexInput input) {
	PixelInput output;

	input.position.w = 1.0f; // Don't mess with the w value, just identity it

	// Project the vertex position onto the world, then view, then projection matrix
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Pixel shader will use this color
	output.color = input.color;

	return output;
}