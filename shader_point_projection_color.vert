// GLSL CODE

/// 1st Vertex Shader

// Projects points to screen space and rotates normal
// stores output on texture

uniform vec3 eye;
uniform int back_face_culling;

varying vec3 normal_vec;
varying vec3 radius_depth_w;

void main(void)
{
  
  vec3 e = (gl_ModelViewProjectionMatrixInverse * vec4(eye, 1.0)).xyz;
  float material = 0.0;

  gl_TexCoord[0] = gl_MultiTexCoord0;
  vec3 unprojected_normal = cross(normalize(gl_TexCoord[0].xyz), normalize(gl_Normal.xyz));

  //if ( (back_face_culling == 1) && (dot(normalize(gl_Vertex.xyz - eye), gl_Normal) < 0.0 )) {
  if ( (back_face_culling == 1) && (dot(normalize(e - gl_Vertex.xyz), unprojected_normal) < 0.0 )) {

    radius_depth_w.x = 0.0;
    
    // for some reason seting the vector to vec4(0.0) drops
    // the performance significantly, at least on the GeForce8800 -- RM 2007-10-19
    gl_Position = vec4(1.0);
  }
  else
    {
      // only rotate point and normal if not culled
      vec4 v = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz, 1.0);
      
      material = gl_TexCoord[0].w;

      // the projected normal is the smaller projection between the minor and major axis
      // compute first the projected axis
      vec3 minor_axis = normalize(gl_NormalMatrix * gl_TexCoord[0].xyz);
      vec3 major_axis = normalize(gl_NormalMatrix * gl_Normal.xyz);
/*       float minor_len = length(minor_axis.xy); */
/*       float major_len = length(major_axis.xy); */
     
      normal_vec = normalize(gl_NormalMatrix * unprojected_normal);

      float radius = gl_Vertex.w;

      if (minor_axis.z > major_axis.z) {
	radius = length(gl_TexCoord[0].xyz);
      }
      else {
	radius = length(gl_Normal.rgb);
      }

      // compute depth value without projection matrix, only modelview
	radius_depth_w = vec3(radius, -(gl_ModelViewMatrix * vec4(gl_Vertex.xyz, 1.0)).z, v.w);
      
      gl_Position = v;
    }
  gl_FrontColor = vec4(0.0, 0.0, 0.0, material);
}
