
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
varying float dist_to_eye;

void main(void)
{  
	gl_TexCoord[0] = gl_MultiTexCoord0;

  //vec3 normal = gl_TexCoord[0].xyz;
  //minor_axis = gl_Normal.xyz;
  //major_axis = gl_TexCoord[0].xyz;
/*	
  if (normal.y > 0.01)
    minor_axis = normalize(cross(vec3(0,0,1), normal));
  else
    minor_axis = normalize(cross(normal, vec3(0,1,0)));
  major_axis = normalize(cross(normal, minor_axis));
*/
	minor_axis = normalize(gl_Normal.xyz);	
	major_axis = normalize(gl_TexCoord[0].xyz);

	vec3 normal = cross(minor_axis, major_axis);

  if ( (back_face_culling == 1) && (normalize(dot((eye - gl_Vertex.xyz), normal)) < -0.1 )) {	
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

      
      minor_length = gl_Vertex.w;
      major_length = gl_TexCoord[0].w;

      major_axis *= major_length;
      minor_axis *= minor_length;

      vec3 e = eye;// - gl_Vertex.xyz;

      float dot_major = dot(normalize(major_axis), normalize(e));
      float dot_minor = dot(normalize(minor_axis), normalize(e));

      float alpha = 2.0*dot_major*dot_minor;
      float den = (minor_length*minor_length + dot_major*dot_major) 
	- (major_length*major_length - dot_minor*dot_minor);  

      alpha /= den;
      alpha = atan(alpha)*0.5;

      float cos_alpha = cos(alpha);
      float sin_alpha = sin(alpha);

      vec3 rot_major;
      rot_major.x = major_axis.x * cos_alpha + minor_axis.x * sin_alpha;
      rot_major.y = major_axis.y * cos_alpha + minor_axis.y * sin_alpha;
      rot_major.z = major_axis.z * cos_alpha + minor_axis.z * sin_alpha;  

      vec3 rot_minor;
      rot_minor.x = -major_axis.x * sin_alpha + minor_axis.x * cos_alpha;
      rot_minor.y = -major_axis.y * sin_alpha + minor_axis.y * cos_alpha;
      rot_minor.z = -major_axis.z * sin_alpha + minor_axis.z * cos_alpha;

      vec3 major_proj = gl_NormalMatrix * rot_major;
      vec3 minor_proj = gl_NormalMatrix * rot_minor;

      major_length = gl_TexCoord[0].w;
      minor_length = major_length * (length(minor_proj.xy) / length(major_proj.xy));
      major_axis = major_proj;
      minor_axis = minor_proj;

 //     if (major_length < minor_length)
	{
	  //major_length = minor_length;	  
//	  minor_length = gl_TexCoord[0].w;
//	  major_length = minor_length * (length(major_proj.xy) / length(minor_proj.xy));
/* 	  major_axis = minor_proj; */
/* 	  minor_axis = major_proj; */
/* 	  major_axis.xy *= -1; */
	}

      major_axis = normalize(major_axis);
      minor_axis = normalize(minor_axis);

      dist_to_eye = length(eye - gl_Vertex.xyz);

      // compute depth value without projection matrix, only modelview
      depth = -(gl_ModelViewMatrix * vec4(gl_Vertex.xyz, 1.0)).z;
      //w = v.w;

      gl_Position = v;
    }
}
