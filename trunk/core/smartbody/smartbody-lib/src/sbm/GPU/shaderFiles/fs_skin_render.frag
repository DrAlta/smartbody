
const vec3 diffuseColor = vec3(1,1,1)*0.6;//vec3(255.f,252.f,181.f)/255.f*vec3(0.8,0.8,0.8)*0.8;
const vec3 specularColor = vec3(101.0/255.0, 101.0/255.0, 101.0/255.0);
const vec3 ambient = vec3(0.0,0.0,0.0);//(vec3(255 + 127, 241, 0 + 2)/255.f)*(vec3(0.2,0.2,0.2));
varying vec3 normal,lightDir[2],halfVector[2];
varying float dist[2];

void main (void)
{  
	vec3 n,halfV;
	float NdotL,NdotHV;
	/* The ambient term will always be present */

	float att;
	vec4 color = vec4(ambient,1.0);
	/* a fragment shader can't write a varying variable, hence we need

	a new variable to store the normalized interpolated normal */
	n = normalize(normal);
	/* compute the dot product between normal and ldir */
	for (int i=0;i<2;i++)
	{
	    att = 1.0/(gl_LightSource[i].constantAttenuation + gl_LightSource[i].linearAttenuation * dist[i] + gl_LightSource[i].quadraticAttenuation * dist[i] * dist[i]);	
		NdotL = max(dot(n,lightDir[i]),0.0);
		if (NdotL > 0.0) {
		color += vec4(diffuseColor*NdotL,0)*att;
		halfV = normalize(halfVector[i]);
		NdotHV = max(dot(n,halfV),0.0);
		color += vec4(specularColor*pow(NdotHV, 30.0),0)*att;			
	}   
	}
    gl_FragColor = color;//vec4(diffuseColor*lambertTerm,1.f);// + vec4(specularColor*specular,1.f);//texel*texel;//textureRect(Transform, coord).x;// + texture2D(HahaTest, coord).x;//vec4(1.0,0.0,0.0,1.0);	   
    //gl_FragColor.xyz = normal;
}
