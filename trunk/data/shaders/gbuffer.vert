/*******************************************************************************
 * GBuffer.vert
 *------------------------------------------------------------------------------
 * Calculates previous and current positions, as well as material attributes.
 ******************************************************************************/
// Framebuffer Outputs
#version 120
varying vec4 vPos;
varying vec3 vNormal;
varying vec2 vTexCoord;

void main()
{
  // Calculations
  vPos = gl_ModelViewMatrix*gl_Vertex;
  gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;


  vNormal = normalize(gl_ModelViewMatrix*vec4(gl_Normal,0.0)).xyz;
  vTexCoord = gl_MultiTexCoord0.xy;  
}
