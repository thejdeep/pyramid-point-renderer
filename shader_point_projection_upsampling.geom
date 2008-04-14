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
  vec3 t;// = vec3(0.0, 0.0, 0.0);
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

  vec3 n = normalize(normal_vec_vertex[0]);

  // early back-face culling discard test
  // simple test for geom max output verts without doing anything
  // it still loses more than 10fps
  if (radius_depth_w_vertex[0].x > 100.0) {
    if (radius_depth_w_vertex[0].x > 0.0) {
      //if (nmax == 0) 
      {
	normal_vec = normalize(gl_NormalMatrix * normal_vec_vertex[0]);
	radius_depth_w = radius_depth_w_vertex[0];
	vec4 proj_v = gl_ModelViewProjectionMatrix * vec4(v.xyz, 1.0);
	
	radius_depth_w.y = -(gl_ModelViewMatrix * vec4(v.xyz, 1.0)).z;
	radius_depth_w.z = proj_v.w;
	
	gl_Position = proj_v;
	gl_FrontColor = gl_FrontColorIn[0];
	EmitVertex();
	EndPrimitive();
      }
    }
  }
  else {

    // find two perpendicular vectors composing an orthonormal
    // basis for the splat
    vec3 perp = normalize(perpendicular(n));

    vec3 minor_axis = perp;
    vec3 major_axis = cross(perp, n);

    minor_axis = normalize (minor_axis);
    major_axis = normalize (major_axis);

    float r = abs(radius_depth_w_vertex[0].x / radius_depth_w_vertex[0].z);
    int nmax = int ( r / max_radius);

    nmax = min(1, nmax);

    normal_vec = normalize(gl_NormalMatrix * normal_vec_vertex[0]);
    radius_depth_w = radius_depth_w_vertex[0];

    if (abs(normal_vec.z) > 0.6)
      nmax = 0;

    if (nmax == 0) {
      vec4 proj_v = gl_ModelViewProjectionMatrix * vec4(v.xyz, 1.0);

      radius_depth_w.y = -(gl_ModelViewMatrix * vec4(v.xyz, 1.0)).z;
      radius_depth_w.z = proj_v.w;

      gl_Position = proj_v;
      gl_FrontColor = gl_FrontColorIn[0];
      EmitVertex();
      EndPrimitive();
    }
    else {
      
      float r = radius_depth_w.x / float(nmax*2+1);      
      radius_depth_w.x /= float(nmax*2+1);

      for (int i = -nmax; i <= nmax; ++i) {
	vec3 step_x = minor_axis * r * i;
	for (int j = -nmax; j <= nmax; ++j) {

	  vec3 step_y = major_axis * r * j;
	  vec4 sub_position = vec4(v.xyz + (step_x+step_y), v.w);
	  vec4 proj_v = gl_ModelViewProjectionMatrix * vec4(sub_position.xyz, 1.0);

	  radius_depth_w.y = -(gl_ModelViewMatrix * vec4(sub_position.xyz, 1.0)).z; 
	  radius_depth_w.z = proj_v.w; 

	  gl_Position = proj_v;
	  gl_FrontColor = gl_FrontColorIn[0];

	  EmitVertex();
	  EndPrimitive();
	}
      }
    }
  }   
}
