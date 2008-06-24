// GLSL CODE

/// 1st Vertex Shader

// Projects points to screen space and rotates normal
// stores output on texture

uniform vec3 eye;
uniform int back_face_culling;

varying vec3 normal_vec_vertex;
varying vec3 radius_depth_w_vertex;

void main(void)
{

  vec3 e = (gl_ModelViewProjectionMatrixInverse * vec4(eye, 1.0)).xyz;

  float material = 0.0;
  gl_TexCoord[0] = gl_MultiTexCoord0;
  vec3 unprojected_normal = cross(normalize(gl_TexCoord[0].xyz), normalize(gl_Normal.xyz));

  //if ( (back_face_culling == 1) && (dot(normalize(gl_Vertex.xyz - eye), gl_Normal) < 0.0 )) {
  if ( (back_face_culling == 1) && (dot(normalize(e - gl_Vertex.xyz), unprojected_normal) < 0.0 )) {

  // back face culling (rotated eye, fixed point)  
    radius_depth_w_vertex.x = 0.0;

    // for some reason seting the vector to vec4(0.0) drops
    // the performance significantly, at least on the GeForce8800 -- RM 2007-10-19
    gl_Position = vec4(1.0);
  }
  else
  {
    // don't transform vertex or normal, leave it for the geometry shader
    vec4 v = vec4(gl_Vertex.xyz, 1.0);
    normal_vec_vertex = unprojected_normal;

    vec3 minor_axis = normalize(gl_NormalMatrix * gl_TexCoord[0].xyz);
    vec3 major_axis = normalize(gl_NormalMatrix * gl_Normal.xyz);

    float radius = gl_Vertex.w;
      
    if (minor_axis.z > major_axis.z) {
      radius = length(gl_TexCoord[0].xyz);
    }
    else {
      radius = length(gl_Normal.rgb);
    }

    radius_depth_w_vertex = vec3(radius, 1.0, v.w); 

    gl_Position = v;
  }

  gl_FrontColor = vec4(0.0, 0.0, 0.0, material);
}
