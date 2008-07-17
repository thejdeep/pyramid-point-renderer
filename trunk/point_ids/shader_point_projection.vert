// GLSL CODE

/// 1st Vertex Shader

// Projects points to screen space and rotates normal
// stores output on texture

uniform vec3 eye;
varying float id;

void main(void)
{
  
  vec3 e = (gl_ModelViewProjectionMatrixInverse * vec4(eye, 1.0)).xyz;

  //if ( (back_face_culling == 1) && (dot(normalize(gl_Vertex.xyz - eye), gl_Normal) < 0.0 )) {
  if (dot(normalize(e - gl_Vertex.xyz), normalize(gl_Normal)) < 0.0 ) {

    id = -1.0;

    // for some reason seting the vector to vec4(0.0) drops
    // the performance significantly, at least on the GeForce8800 -- RM 2007-10-19
    gl_Position = vec4(0.5, 0.5, 0.5, 1.0);
  }
  else
  {
    // only rotate point and normal if not culled
    vec4 v = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz, 1.0);
   
    id = gl_Vertex.w;

    gl_Position = v;
  }
}
