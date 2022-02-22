struct PS_INPUT {
	float4 Position : SV_POSITION;
	float4 Color : COLOR0;
};

struct PS_OUTPUT {
	float4 RGB_Color : SV_TARGET;
};

PS_OUTPUT main(PS_INPUT In) {
	PS_OUTPUT Output;
	Output.RGB_Color = In.Color;
	return Output;
}