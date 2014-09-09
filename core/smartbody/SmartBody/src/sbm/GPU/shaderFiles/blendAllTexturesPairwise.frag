uniform int			uNumberOfTextures;
uniform int			uIteration;
uniform float		uWeight;
uniform float		uTotalWeights;
uniform sampler2D	uNeutralSampler;			// Neutral texture
uniform	sampler2D	uExpressionSampler;			// Current expresion texture
uniform sampler2D	uPreviousResultSampler;		// Result sampler	

void main()
{
		vec4 new			= vec4(0.0, 0.0, 0.0, 1.0);
		
		vec4 texNeutral		= texture2D(uNeutralSampler, gl_TexCoord[0].st);
		vec4 texExpression	= texture2D(uExpressionSampler, gl_TexCoord[0].st);
		vec4 texPrevious	= texture2D(uPreviousResultSampler,  gl_TexCoord[0].st);
		vec4 texBlend		= texNeutral * (1.0 - uWeight) + texExpression * uWeight;

		if(uTotalWeights > 0.0001)
		{
			// The C++ loop that calls this shaders runs
			// from (uNumberOfTextures - 1) to 0, so this is the first iteration
			if(uIteration == uNumberOfTextures-1)	
			{
				new	= uWeight/uTotalWeights * texBlend;
			}
			// If not the first iteration, updates previous blend
			else
			{
				new	= texPrevious + uWeight/uTotalWeights * texBlend;
			}
		}
		// If not weights, output neutral expression 
		else
		{
			new = texNeutral;
		}
		gl_FragColor		= new;
}
