cbuffer ModelViewProjectionConstantBuffer : register(b0) {
	matrix world;
	matrix view;
	matrix projection;
};

struct VS_INPUT {
	float3 v_pos : POSITION;
	float3 v_color : COLOR0;
};

struct PS_INPUT {
	float4 Position : SV_POSITION;
	float4 Color : COLOR0;
};

PS_INPUT main(VS_INPUT input) {
	PS_INPUT output;
	float4 pos = float4(input.v_pos, 1.0f);
	// Transform position from object space to homogenous projection space
	pos = mul(pos, world);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.Position = pos;
	// Just pass through the color data
	output.Color = float4(input.v_color, 1.0f);
	return output;
}