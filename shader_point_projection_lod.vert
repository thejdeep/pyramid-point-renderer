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

varying out float radius_ratio;

void main() {

  gl_FrontColor = gl_Color;

  if ( dot(normalize(eye - gl_Vertex.xyz), gl_Normal) < 0.00 ) {

    radius_depth_w_vertex = vec3(0.0);
    gl_Position = vec4(1.0);

  }
  else {

    vec4 v = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz, 1.0);
    vec4 orig_v = gl_Vertex;

    radius_depth_w_vertex = vec3(orig_v.w, -(gl_ModelViewMatrix * vec4(orig_v.xyz, 1.0)).z, v.w);
    normal_vec_vertex = normalize(gl_NormalMatrix * gl_Normal);

    radius_ratio = orig_v.w / v.w;

    gl_Position = v;

  }

}
