/**
 *   GLSL Geometry Shader
 *
 *   File: geometry.shader
 *
 *  Author: Ricardo Marroquim - Date: 05-Mar-2008
 *
 **/

//--- Definitions ---
#version 120

#extension GL_EXT_geometry_shader4 : enable

#extension GL_EXT_gpu_shader4 : enable

uniform float max_radius;
uniform int num_subdivisions;

const int max_num_subdivisions = 8;

vec2 desloc[max_num_subdivisions] = 
  vec2[max_num_subdivisions](vec2( 1.0,  1.0),
			     vec2(-1.0, -1.0),
			     vec2( 1.0, -1.0),
			     vec2(-1.0,  1.0),
			     vec2( 0.0, -1.0),
			     vec2( 0.0,  1.0),
			     vec2(-1.0,  0.0),
			     vec2( 1.0,  0.0));

//--- Varyings ---
varying out vec3 normal_vec;
varying out vec3 radius_depth_w;

varying in vec3 normal_vec_vertex[1];
varying in vec3 radius_depth_w_vertex[1];

void main() {

  vec4 v = gl_PositionIn[0];

  if (radius_depth_w_vertex[0].x > 0) {
    float r = abs(radius_depth_w_vertex[0].x / radius_depth_w_vertex[0].z);
    if ((r <= max_radius) || (num_subdivisions == 1)) {
      normal_vec = normal_vec_vertex[0];
      radius_depth_w = radius_depth_w_vertex[0];
      gl_Position = v;
      EmitVertex();
      EndPrimitive();
    }
    else {
      normal_vec = normal_vec_vertex[0];
      radius_depth_w = radius_depth_w_vertex[0];
      radius_depth_w.x *= 0.6;
      for (int i = 0; i < num_subdivisions; ++i) {
	vec4 sub_position = vec4(v.xy + radius_depth_w_vertex[0].x * desloc[i].xy, v.zw);
	gl_Position = sub_position;
	EmitVertex();
	EndPrimitive();
      }
    }    
  }
}
