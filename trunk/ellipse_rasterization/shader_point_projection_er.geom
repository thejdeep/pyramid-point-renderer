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

//--- Varyings ---
varying out vec3 normal_vec;
varying out vec3 radius_depth_w;

varying in vec3 normal_vec_vertex[1];
varying in vec3 radius_depth_w_vertex[1];

/* vec2 desloc[4] = vec2[4](vec2( 1.0,  1.0), */
/* 			 vec2( 1.0, -1.0), */
/* 			 vec2(-1.0, -1.0), */
/* 			 vec2(-1.0,  1.0)); */

vec3 perpendicular( in vec3 v ) {
  vec3 t = vec3(0.0, 0.0, 0.0);
  // select the shortest of projections of axes on v
  // (the closest to perpendicular to v),
  // and project it to the plane defined by v

  if( abs(v.x) < abs(v.y) ) { // x < y
    if( abs(v.x) < abs(v.z) ) { // x < y && x < z
      t = vec3 (1.0 - v.x * v.x, -v.x * v.y, -v.x * v.z);
      return t;
    }
  }
  else { // y <= x
    if( abs(v.y) < abs(v.z) ) { // y <= x && y < z
      t = vec3( -v.y * v.x, 1.0 - v.y * v.y, -v.y * v.z);
      return t;
    }
  }

  // z <= x && z <= y
  t = vec3(-v.z * v.x, -v.z * v.y, 1.0 - v.z * v.z);
  return t;
}

void main() {

  vec4 v = gl_PositionIn[0];

  vec3 n = normal_vec_vertex[0];

  // find two perpendicular vectors composing an orthonormal
  // basis for the splat
  float r = abs(radius_depth_w_vertex[0].x / radius_depth_w_vertex[0].z);
  vec3 perp = perpendicular(n);

  vec3 minor_axis = perp * r;
  vec3 major_axis = cross(perp, n) * r;

  minor_axis = normalize (minor_axis);
  major_axis = normalize (major_axis);

  int nmax = int ( r / max_radius);

  nmax = min(2, num_subdivisions);

  if ((nmax == 0) || (num_subdivisions == 1)) {
    normal_vec = normal_vec_vertex[0];
    radius_depth_w = radius_depth_w_vertex[0];
    gl_Position = v;
    EmitVertex();
    EndPrimitive();
  }
  else {
    normal_vec = normal_vec_vertex[0];
    radius_depth_w = radius_depth_w_vertex[0];
    radius_depth_w.x /= float(nmax);
    for (int i = -nmax; i <= nmax; ++i) {
      vec3 step_x = minor_axis * radius_depth_w.x * i;
      for (int j = -nmax; j <= nmax; ++j) {
	vec3 step_y = major_axis * radius_depth_w.x * j;
	vec4 sub_position = vec4(v.xyz + (step_x+step_y), v.w);
	gl_Position = sub_position;
	EmitVertex();
	EndPrimitive();
      }
    }
  }
    

/*   if (radius_depth_w_vertex[0].x > 0) { */
/*     float r = abs(radius_depth_w_vertex[0].x / radius_depth_w_vertex[0].z); */
/*     if ((r <= max_radius) || (num_subdivisions == 1)) { */
/*       normal_vec = normal_vec_vertex[0]; */
/*       radius_depth_w = radius_depth_w_vertex[0]; */
/*       gl_Position = v; */
/*       EmitVertex(); */
/*       EndPrimitive(); */
/*     } */
/*     else { */
/*       normal_vec = normal_vec_vertex[0]; */
/*       radius_depth_w = radius_depth_w_vertex[0]; */
/*       radius_depth_w.x *= 0.6; */
/*       for (int i = 0; i < num_subdivisions; ++i) { */
/* 	vec4 sub_position = vec4(v.xy + radius_depth_w_vertex[0].x * desloc[i].xy * 2.0, v.zw); */
/* 	gl_Position = sub_position; */
/* 	EmitVertex(); */
/* 	EndPrimitive(); */
/*       } */
/*     } */
/*   } */
}
