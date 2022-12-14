#version 120
/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2009 Torus Knot Software Ltd
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

//-----------------------------------------------------------------------------
// Program Name: FFPLib_Lighting
// Program Desc: Lighting functions of the FFP.
// Program Type: Vertex shader
// Language: GLSL
// Notes: Implements core functions for FFPLighting class.
// based on lighting engine. 
// See http://msdn.microsoft.com/en-us/library/bb147178.aspx
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void FFP_Light_Directional_Diffuse(in mat4 mWorldViewIT, 
				   in vec3 vNormal,
				   in vec3 vNegLightDirView,
				   in vec3 vDiffuseColour, 
				   in vec3 vBaseColour, 
				   out vec3 vOut)
{
	vec3 vNormalView = normalize((mWorldViewIT * vec4(vNormal.xyz, 1.0)).xyz); 
	float nDotL = dot(vNormalView, vNegLightDirView);
	
	vOut = vBaseColour + vDiffuseColour * clamp(nDotL, 0.0, 1.0);
}


//-----------------------------------------------------------------------------
void FFP_Light_Directional_DiffuseSpecular(in mat4 mWorldView, 
					in vec4 vPos,
					in mat4 mWorldViewIT, 
					in vec3 vNormal,
					in vec3 vNegLightDirView,
					in vec3 vDiffuseColour, 
					in vec3 vSpecularColour, 
					in float fSpecularPower, 
					in vec3 vBaseDiffuseColour,
					in vec3 vBaseSpecularColour,					
					out vec3 vOutDiffuse,
					out vec3 vOutSpecular)
{
	vOutDiffuse  = vBaseDiffuseColour;
	vOutSpecular = vBaseSpecularColour;
	
	vec3 vNormalView = normalize((mWorldViewIT * vec4(vNormal.xyz, 1.0)).xyz); 	
	float nDotL		   = dot(vNormalView, vNegLightDirView);			
	vec3 vView       = -normalize((mWorldView* vPos).xyz);
	vec3 vHalfWay    = normalize(vView + vNegLightDirView);
	float nDotH        = dot(vNormalView, vHalfWay);
	
	if (nDotL > 0.0)
	{
		vOutDiffuse  += vDiffuseColour * nDotL;		
		vOutSpecular += vSpecularColour * pow(clamp(nDotH, 0.0, 1.0), fSpecularPower);						
	}
}


//-----------------------------------------------------------------------------
void FFP_Light_Point_Diffuse(in mat4 mWorldView, 
					in vec4 vPos,
					in mat4 mWorldViewIT, 
				    in vec3 vNormal,
				    in vec3 vLightPosView,
				    in vec4 vAttParams,
				    in vec3 vDiffuseColour, 
				    in vec3 vBaseColour, 
				    out vec3 vOut)
{
	vOut = vBaseColour;		

	vec3 vViewPos    = (mWorldView * vPos).xyz;
	vec3 vLightView  = vLightPosView - vViewPos;
	float fLightD      = length(vLightView);
	vec3 vNormalView = normalize((mWorldViewIT * vec4(vNormal.xyz, 1.0)).xyz); 
	float nDotL        = dot(vNormalView, normalize(vLightView));
	
	if (nDotL > 0.0 && fLightD <= vAttParams.x)
	{
		float fAtten	   = 1.0 / (vAttParams.y + vAttParams.z*fLightD + vAttParams.w*fLightD*fLightD);
			
		vOut += vDiffuseColour * nDotL * fAtten;
	}		
}

//-----------------------------------------------------------------------------
void FFP_Light_Point_DiffuseSpecular(in mat4 mWorldView, 
					in vec4 vPos,
					in mat4 mWorldViewIT, 
				    in vec3 vNormal,
				    in vec3 vLightPosView,
				    in vec4 vAttParams,
				    in vec3 vDiffuseColour, 
				    in vec3 vSpecularColour, 
					in float fSpecularPower, 
				    in vec3 vBaseDiffuseColour,
					in vec3 vBaseSpecularColour,					
					out vec3 vOutDiffuse,
					out vec3 vOutSpecular)
{
	vOutDiffuse  = vBaseDiffuseColour;
	vOutSpecular = vBaseSpecularColour;

	vec3 vViewPos    = (mWorldView * vPos).xyz;
	vec3 vLightView  = vLightPosView - vViewPos;
	float fLightD      = length(vLightView);
	
	vLightView		   = normalize(vLightView);	
	vec3 vNormalView = normalize((mWorldViewIT * vec4(vNormal.xyz, 1.0)).xyz); 
	float nDotL        = dot(vNormalView, vLightView);	
		
	if (nDotL > 0.0 && fLightD <= vAttParams.x)
	{					
		vec3 vView       = -normalize(vViewPos);			
		vec3 vHalfWay    = normalize(vView + vLightView);		
		float nDotH        = dot(vNormalView, vHalfWay);
		float fAtten	   = 1.0 / (vAttParams.y + vAttParams.z*fLightD + vAttParams.w*fLightD*fLightD);					
		
		vOutDiffuse  += vDiffuseColour * nDotL * fAtten;
		vOutSpecular += vSpecularColour * pow(clamp(nDotH, 0.0, 1.0), fSpecularPower) * fAtten;					
	}		
}

//-----------------------------------------------------------------------------
void FFP_Light_Spot_Diffuse(in mat4 mWorldView, 
					in vec4 vPos,
					in mat4 mWorldViewIT, 
				    in vec3 vNormal,
				    in vec3 vLightPosView,
				    in vec3 vNegLightDirView,
				    in vec4 vAttParams,
				    in vec3 vSpotParams,
				    in vec3 vDiffuseColour, 
				    in vec3 vBaseColour, 
				    out vec3 vOut)
{
	vOut = vBaseColour;		

	vec3 vViewPos    = (mWorldView * vPos).xyz;
	vec3 vLightView  = vLightPosView - vViewPos;
	float fLightD      = length(vLightView);
	vLightView		   = normalize(vLightView);
	vec3 vNormalView = normalize((mWorldViewIT * vec4(vNormal.xyz, 1.0)).xyz); 	
	float nDotL        = dot(vNormalView, vLightView);
	
	if (nDotL > 0.0 && fLightD <= vAttParams.x)
	{
		float fAtten	= 1.0 / (vAttParams.y + vAttParams.z*fLightD + vAttParams.w*fLightD*fLightD);
		float rho		= dot(vNegLightDirView, vLightView);						
		float fSpotE	= clamp((rho - vSpotParams.y) / (vSpotParams.x - vSpotParams.y), 0.0, 1.0);
		float fSpotT	= pow(fSpotE, vSpotParams.z);	
						
		vOut += vDiffuseColour * nDotL * fAtten * fSpotT;
	}		
}

//-----------------------------------------------------------------------------
void FFP_Light_Spot_DiffuseSpecular(in mat4 mWorldView, 
					in vec4 vPos,
					in mat4 mWorldViewIT, 
				    in vec3 vNormal,
				    in vec3 vLightPosView,
				    in vec3 vNegLightDirView,
				    in vec4 vAttParams,
				    in vec3 vSpotParams,
				    in vec3 vDiffuseColour, 
				    in vec3 vSpecularColour, 
					in float fSpecularPower, 
				    in vec3 vBaseDiffuseColour,
					in vec3 vBaseSpecularColour,					
					out vec3 vOutDiffuse,
					out vec3 vOutSpecular)
{
	vOutDiffuse  = vBaseDiffuseColour;		
	vOutSpecular = vBaseSpecularColour;

	vec3 vViewPos    = (mWorldView * vPos).xyz;
	vec3 vLightView  = vLightPosView - vViewPos;
	float fLightD      = length(vLightView);
	vLightView		   = normalize(vLightView);
	vec3 vNormalView = normalize((mWorldViewIT * vec4(vNormal.xyz, 1.0)).xyz); 	
	float nDotL        = dot(vNormalView, vLightView);
	
	
	
	if (nDotL > 0.0 && fLightD <= vAttParams.x)
	{
		vec3 vView       = -normalize(vViewPos);	
		vec3 vHalfWay    = normalize(vView + vLightView);				
		float nDotH        = dot(vNormalView, vHalfWay);
		float fAtten	= 1.0 / (vAttParams.y + vAttParams.z*fLightD + vAttParams.w*fLightD*fLightD);
		float rho		= dot(vNegLightDirView, vLightView);						
		float fSpotE	= clamp((rho - vSpotParams.y) / (vSpotParams.x - vSpotParams.y), 0.0, 1.0);
		float fSpotT	= pow(fSpotE, vSpotParams.z);	
						
		vOutDiffuse  += vDiffuseColour * nDotL * fAtten * fSpotT;
		vOutSpecular += vSpecularColour * pow(clamp(nDotH, 0.0, 1.0), fSpecularPower) * fAtten * fSpotT;
	}		
}
