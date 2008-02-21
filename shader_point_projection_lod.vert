/**
 *   GLSL Vertex Shader
 *
 *   File: vertex.shader
 *
 *  Author: Andre Maximo - Date: Nov 21, 2007
 *
 **/ 

//--- Definitions ---
#version 120

#extension GL_EXT_gpu_shader4 : enable

//--- Uniforms ---
uniform vec3 eye;

//--- Varyings ---
varying out vec3 normal_vec_vertex;
varying out vec3 radius_depth_w_vertex;

varying out float ep;

void main() {

  gl_TexCoord[0] = gl_MultiTexCoord0;

  normal_vec_vertex = gl_Color.xyz;

  // angle between view vector and normal
  float cos_alpha = dot(normalize(gl_Vertex.xyz - eye), normal_vec_vertex);
  if ( cos_alpha < -1.20 ) {

    radius_depth_w_vertex.x = 0.0;
    gl_Position = vec4(1.0);

  }
  else {

    vec4 v = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz, 1.0);

    radius_depth_w_vertex = vec3(gl_Vertex.w, -(gl_ModelViewMatrix * vec4(gl_Vertex.xyz, 1.0)).z, v.w);
    normal_vec_vertex = normalize(gl_NormalMatrix * normal_vec_vertex);

    // project perpendicular error
    ep = gl_Color.w;
    float sin_alpha = sqrt (1.0 - cos_alpha*cos_alpha);
    //float sin_alpha = sin(acos(cos_alpha));
    //vec3 orig_eye = vec3(0.0, 0.0, -3.0);
    //float d = v.w;
    float d = length(eye - gl_Vertex.xyz);
    ep *= sin_alpha / d;

    gl_Position = v;

  }

}
