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

//const float epsilon = 0.02;
const float epsilon = 0.006;

//--- Uniforms ---
uniform samplerBuffer vertex_buffer;
uniform samplerBuffer normal_buffer;
uniform int color_per_lod;

//--- Varyings ---
varying out vec3 normal_vec;
varying out vec3 radius_depth_w;

varying in vec3 normal_vec_vertex[1];
varying in vec3 radius_depth_w_vertex[1];

varying in float ep[1];

vec4 lodColors [4] = vec4[4] ( vec4(1.0, 0.0, 0.0, 0.0),
			       vec4(0.0, 1.0, 0.0, 0.2),
			       vec4(0.0, 0.0, 1.0, 0.4),
			       vec4(0.3, 0.3, 0.0, 0.6));

vec4 black = vec4(0.0, 0.0, 0.0, 0.0);

void main() {

  vec4 v = gl_PositionIn[0];
  ivec4 surfels_per_level = ivec4(gl_TexCoordIn[0][0]);

  if (radius_depth_w_vertex[0].x > 0.0)
    {   
      
      if ( ep[0] < 1.0*epsilon ) {

	radius_depth_w = radius_depth_w_vertex[0];
	normal_vec = normal_vec_vertex[0];

	if (color_per_lod > 0)
	  gl_FrontColor = lodColors[3];	
	else
	  gl_FrontColor = black;
	gl_Position = gl_PositionIn[0];
	EmitVertex();
	EndPrimitive();

      }
      else {
	vec4 color = vec4(1.0);

	int lod_id = surfels_per_level.x;
	int num_surfels = 0;

	if ( ep[0] < 4.0*epsilon ) {
	  num_surfels = surfels_per_level.y;
	  color = lodColors[2];
	}
	else if ( ep[0] < 8.0*epsilon ) {
	  lod_id += surfels_per_level.y;
	  num_surfels = surfels_per_level.z;
	  color = lodColors[1];
	}
	else {
	  lod_id += surfels_per_level.y + surfels_per_level.z;
	  num_surfels = surfels_per_level.w;
	  color = lodColors[0];
	}
           
	for (int i = 0; i < num_surfels; ++i) {

	  vec4 v = texelFetchBuffer(vertex_buffer, lod_id + i).xyzw;

	  if (v.w > 0.0) 
	    {

	      normal_vec = texelFetchBuffer(normal_buffer, lod_id + i).xyz;

	      vec4 orig_v = v;
	      v = gl_ModelViewProjectionMatrix * vec4(v.xyz, 1.0);

	      radius_depth_w = vec3(orig_v.w, -(gl_ModelViewMatrix * vec4(orig_v.xyz, 1.0)).z, v.w);
	      normal_vec = normalize(gl_NormalMatrix * normal_vec);

	      if (color_per_lod > 0)
		gl_FrontColor = color;
	      else
		gl_FrontColor = black;
	      gl_Position = v;
	      EmitVertex();
	      EndPrimitive();	    
	    }

	}

      }

    }
}
