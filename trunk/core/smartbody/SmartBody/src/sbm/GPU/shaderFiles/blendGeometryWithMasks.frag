#version 150

#define MAX_SHAPES 14

uniform sampler2D	uNeutralSampler[MAX_SHAPES];
uniform int			uNumberOfShapes;
uniform float		uWeights[MAX_SHAPES];
uniform bool		uShowMasks;
in vec2				texCoords;
out vec4			final_color;

void main()
{
	vec4 tex[MAX_SHAPES];

	tex[0]	= texture2D(uNeutralSampler[0], texCoords);
	tex[1]	= texture2D(uNeutralSampler[1], texCoords);
	tex[2]	= texture2D(uNeutralSampler[2], texCoords);
	tex[3]	= texture2D(uNeutralSampler[3], texCoords);
	tex[4]	= texture2D(uNeutralSampler[4], texCoords);
	tex[5]	= texture2D(uNeutralSampler[5], texCoords);
	tex[6]	= texture2D(uNeutralSampler[6], texCoords);
	tex[7]	= texture2D(uNeutralSampler[7], texCoords);
	tex[8]	= texture2D(uNeutralSampler[8], texCoords);
	tex[9]	= texture2D(uNeutralSampler[9], texCoords);
	tex[10] = texture2D(uNeutralSampler[10], texCoords);
	tex[11] = texture2D(uNeutralSampler[11], texCoords);
	tex[12] = texture2D(uNeutralSampler[12], texCoords);
	tex[13] = texture2D(uNeutralSampler[13], texCoords);

	final_color					= vec4(0.0f, 0.0f, 0.0f, 1.0f);
	float interpolatedWeight	= 0.0;
	float interpolatedWeights[MAX_SHAPES];

	bool uShowWeights = uShowMasks;

	if(uShowWeights)
	{
		tex[0] = vec4(0.0, 0.0, 0.0, 1.0);
	}

	// If more than 1 shape used, need to blend textures
	if(uNumberOfShapes > 1)
	{
		float totalWeights				= 0.0;
		float totalInterpolatedWeights	= 0.0;
	
		// Per-shape interpolation with neutral
		for(int i=1; i < uNumberOfShapes; i++)
		{
			interpolatedWeights[i] = 0.0;

			// Interpolates blending weight by masking (encoded in alpha channel)
			interpolatedWeight = float(uWeights[i]) * float(tex[i].a);
			interpolatedWeights[i] = interpolatedWeight;

			if(uShowWeights)
			{
				tex[i] = vec4(0.0f, interpolatedWeight, 0.0f, 1.0f);
			}

			tex[i] = tex[0] * (1.0 - interpolatedWeight) + tex[i] * interpolatedWeight;

			totalWeights				= totalWeights + uWeights[i];
			totalInterpolatedWeights	= totalInterpolatedWeights + interpolatedWeight;
		}


		if(totalInterpolatedWeights < 0.0001)
		{
			final_color = tex[0];
		}
		else
		{
			// Normalization of weights wrt sum of weights
			for(int i=1; i < uNumberOfShapes; i++)
			{
				final_color = final_color + vec4(tex[i].rgb * (interpolatedWeights[i] / totalInterpolatedWeights), 1.0f);
			}
		}
	}
	//	All weights are 0, render neutral
	else
	{
		final_color =  tex[0];
	}
}