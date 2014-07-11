//#version 140
//
//uniform sampler2D tex0;
//uniform sampler2D tex1;
//uniform float weight;
//
//in vec2 out_texCoord;
//out	vec4 final_color;
//
//
//void main()
//{
//	float clamp_weight	= clamp(weight, 0.0, 1.0);
//	vec4 tex0			= texture(tex0,  out_texCoord);
//	vec4 tex1			= texture(tex1,  out_texCoord);
//	final_color			= (1.0 - clamp_weight) * tex0 + clamp_weight * tex1;
//}

uniform sampler2D	texNeutral;
uniform sampler2D	texFv;
uniform sampler2D	texOpen;
uniform sampler2D	texPBM;
uniform sampler2D	texShCh;
uniform sampler2D	texW;
uniform sampler2D	texWide;
					
uniform float		wNeutral;
uniform float		wFv;
uniform float		wOpen;
uniform float		wPBM;
uniform float		wShCh;
uniform float		wW;
uniform float		wWide;

void main()
{

		vec4 neutral	= texture2D(texNeutral, gl_TexCoord[0].st);
		vec4 fv			= texture2D(texFv, gl_TexCoord[0].st);
		vec4 open		= texture2D(texOpen, gl_TexCoord[0].st);
		vec4 PBM		= texture2D(texPBM, gl_TexCoord[0].st);
		vec4 ShCh		= texture2D(texShCh, gl_TexCoord[0].st);
		vec4 W			= texture2D(texW, gl_TexCoord[0].st);
		vec4 Wide		= texture2D(texWide, gl_TexCoord[0].st);

		vec4 new		= vec4(0.0, 0.0, 0.0, 1.0);


		//	2nd attempt

		vec4 blendFv	= neutral * (1.0 - wFv) + fv * wFv;
		vec4 blendOpen	= neutral * (1.0 - wOpen) + open * wOpen;
		vec4 blendPBM	= neutral * (1.0 - wPBM) + PBM * wPBM;
		vec4 blendShCh	= neutral * (1.0 - wShCh) + ShCh * wShCh;
		vec4 blendW		= neutral * (1.0 - wW) + W * wW;
		vec4 blendWide	= neutral * (1.0 - wWide) + Wide *wWide;

		float total_weights = wFv + wOpen + wPBM + wShCh + wW + wWide;

		if(total_weights < 0.0001)
		{
			new		= neutral;
		} 
		else
		{
			new		+=  wFv/total_weights * blendFv;
			new		+= wOpen/total_weights * blendOpen;
			new		+= wPBM/total_weights * blendPBM;
			new		+= wShCh/total_weights * blendShCh;
			new		+= wW/total_weights * blendW;
			new		+= wWide/total_weights * blendWide;
		}


		// 1st attempt
		//new		= texture2D(texNeutral, gl_TexCoord[0].st);
		//if(wFv > 0.0001)
		//{
		//	new	= new + (fv - neutral) * wFv;
		//}
		//if(wOpen > 0.0001)
		//{	
		//	new	= new + (open - neutral) * wOpen;
		//}
		//if(wPBM > 0.0001)
		//{
		//	new	= new + (PBM - neutral) * wPBM;
		//}
		//if(wShCh > 0.0001)
		//{
		//	new	= new + (ShCh - neutral) * wShCh;
		//}		
		//if(wW > 0.0001)
		//{
		//	new	= new + (W - neutral) * wW;
		//}		
		//if(wWide > 0.0001)
		//{
		//	new	= new + (Wide - neutral) * wWide;
		//}	


		gl_FragColor = new; 

}
