// GLSL CODE

/// 1st Vertex Shader

// Projects points to screen space and rotates normal
// stores output on texture

uniform vec3 eye;
uniform int back_face_culling;

varying vec3 minor_axis;
varying vec3 major_axis;
varying float minor_length;
varying float major_length;
varying float depth;
varying float w;
varying float dist_to_eye;

void main(void)
{  
	minor_axis = gl_Normal.xyz;
	major_axis = gl_TexCoord[0].xyz;

	if ( (back_face_culling == 1) && (normalize(dot((eye - gl_Vertex.xyz), cross(major_axis, minor_axis))) < -0.1 )) {

    	minor_length = 0.0;
    	major_length = 0.0;
    
    	// for some reason seting the vector to vec4(0.0) drops
    	// the performance significantly, at least on the GeForce8800 -- RM 2007-10-19
    	gl_Position = vec4(1.0);
  	}
	else
	{
    	// only rotate point and normal if not culled
      	vec4 v = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz, 1.0);           

		gl_TexCoord[0] = gl_MultiTexCoord0;
		
		float minor_length = gl_Vertex.w;
		float major_length = gl_TexCoord[0].w;		

		minor_axis = normalize(gl_NormalMatrix * minor_axis);
    	major_axis = normalize(gl_NormalMatrix * major_axis);

	  	dist_to_eye = length(eye - gl_Vertex.xyz);

      	// compute depth value without projection matrix, only modelview
      	depth = -(gl_ModelViewMatrix * vec4(gl_Vertex.xyz, 1.0)).z;
      	w = v.w;
      
      	gl_Position = v;
	}
}
