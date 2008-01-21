/**
 *   GLSL Geometry Shader
 *
 *   File: geometry.shader
 *
 *  Author: Andre Maximo - Date: Dec 3, 2007
 *
 **/

//--- Definitions ---
#version 120

#extension GL_EXT_geometry_shader4 : enable

#extension GL_EXT_gpu_shader4 : enable

const float pixel_size = 2.0 / 1024.0;

//--- Uniforms ---
uniform samplerBuffer vertex_buffer;
uniform samplerBuffer normal_buffer;

uniform int total_surfels;

//--- Varyings ---
varying out vec3 normal_vec;
varying out vec3 radius_depth_w;

varying in vec3 normal_vec_vertex[1];
varying in vec3 radius_depth_w_vertex[1];

varying in float radius_ratio[1];

void main() {

  vec4 v = gl_PositionIn[0];

/*   if ( dot(normalize(eye - v.xyz), n) < 0.00 ) { */
/*     radius_depth_w = vec3(0.0); */
/*     gl_Position = vec4(1.0); */
/*   } */
/*   else   */
  if (radius_depth_w_vertex[0].x > 0.0)
  {   
/*     vec4 orig_v = v; */
/*     v = gl_ModelViewProjectionMatrix * vec4(v.xyz, 1.0); */
/*     float r = orig_v.w / v.w; */

/*     if (r < 3*pixel_size) { */
/*       radius_depth_w = vec3(orig_v.w, -(gl_ModelViewMatrix * vec4(orig_v.xyz, 1.0)).z, v.w); */
/*       normal_vec = normalize(gl_NormalMatrix * n); */
    if ( radius_ratio[0] < 3*pixel_size ) {

      radius_depth_w = radius_depth_w_vertex[0];
      normal_vec = normal_vec_vertex[0];

      gl_Position = gl_PositionIn[0];
      EmitVertex();
      EndPrimitive();

    }
    else {

      vec4 surfels_per_level = gl_FrontColorIn[0].rgba;
      surfels_per_level *= vec4(1.0, 4.0, 16.0, 64.0);

      int lod_id = int(surfels_per_level.x * total_surfels);
      int num_surfels = 0;

      if (radius_ratio[0] < 6*pixel_size) {

	num_surfels = int(surfels_per_level.y);

      } else if (radius_ratio[0] < 9*pixel_size) {

        lod_id += int(surfels_per_level.y);
	num_surfels = int(surfels_per_level.z);

      }
      else {

	lod_id += int(surfels_per_level.y + surfels_per_level.z);
	num_surfels = int(surfels_per_level.w);

      }
           
      for (int i = 0; i < num_surfels; ++i) {

	vec4 v = texelFetchBuffer(vertex_buffer, lod_id + i).xyzw;

	if (v.w > 0.0) {

	  normal_vec = texelFetchBuffer(normal_buffer, lod_id + i).xyz;

	  vec4 orig_v = v;
	  v = gl_ModelViewProjectionMatrix * vec4(v.xyz, 1.0);

	  radius_depth_w = vec3(orig_v.w, -(gl_ModelViewMatrix * vec4(orig_v.xyz, 1.0)).z, v.w);
	  normal_vec = normalize(gl_NormalMatrix * normal_vec);

	  gl_Position = v;
	  EmitVertex();
	  EndPrimitive();

	}

      }

    }

  }

}
