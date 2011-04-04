
// uniform vec3 diffuseColor = vec3(255.f,252.f,181.f)/255.f*vec3(0.8,0.8,0.8)*0.8;
// uniform vec3 specularColor = vec3(101.f, 101.f, 101.f)/255.f ;
// uniform vec3 ambient = (vec3(255 + 127, 241, 0 + 2)/255.f)*(vec3(0.2,0.2,0.2));
// uniform float bWire = 1.f;

//varying vec3 normal,lightDir,eyeVec;

varying vec3 OutNormal;

void main (void)
{  
//    float shine = 30.f;
//    vec3 N = normalize(normal);
//    vec3 L = normalize(lightDir);
//    float lambertTerm = dot(N,L);
	vec3 L = vec3(0,1,0);
	float lambert = dot(OutNormal,L);

//    vec3 E = normalize(eyeVec);
//    vec3 R = reflect(-L, N);
//    float specular = pow( max(dot(R, E), 0.0),shine);
//    if (bWire)
// 	   gl_FragColor.rgb = 0;
//    else
   gl_FragColor = vec4(lambert,lambert,lambert,1.f) ; //vec4(diffuseColor*lambertTerm,1.f);// + vec4(specularColor*specular,1.f);//texel*texel;//textureRect(Transform, coord).x;// + texture2D(HahaTest, coord).x;//vec4(1.0,0.0,0.0,1.0);	   
   //gl_FragColor.xyz = normal;
}
