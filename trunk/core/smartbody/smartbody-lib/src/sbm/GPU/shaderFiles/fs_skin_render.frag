
uniform vec3 diffuseColor = vec3(255.f,252.f,181.f)/255.f*vec3(0.8,0.8,0.8)*0.8;
uniform vec3 specularColor = vec3(101.f, 101.f, 101.f)/255.f ;
uniform vec3 ambient = (vec3(255 + 127, 241, 0 + 2)/255.f)*(vec3(0.2,0.2,0.2));
varying vec3 normal,lightDir,halfVector;

void main (void)
{  
	vec3 n,halfV;
	float NdotL,NdotHV;
	/* The ambient term will always be present */

	vec4 color = vec4(ambient,1.0);
	/* a fragment shader can't write a varying variable, hence we need

	a new variable to store the normalized interpolated normal */
	n = normalize(normal);
	/* compute the dot product between normal and ldir */

	NdotL = max(dot(n,lightDir),0.0);
   if (NdotL > 0.0) {
		color += vec4(diffuseColor*NdotL,0);
		halfV = normalize(halfVector);
		NdotHV = max(dot(n,halfV),0.0);
		color += vec4(specularColor*pow(NdotHV, 30),0);
	}
   gl_FragColor = color;//vec4(diffuseColor*lambertTerm,1.f);// + vec4(specularColor*specular,1.f);//texel*texel;//textureRect(Transform, coord).x;// + texture2D(HahaTest, coord).x;//vec4(1.0,0.0,0.0,1.0);	   
   //gl_FragColor.xyz = normal;
}
