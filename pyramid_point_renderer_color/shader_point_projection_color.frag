/// GLSL CODE

/// 1st Fragment Shader

// Projects points to screen space and rotates normal
// stores output on texture
#extension GL_ARB_draw_buffers : enable
//#extension GL_EXT_gpu_shader4 : enable

// height of near viewing plane
//uniform float h_near;

varying vec3 normal_vec;
varying vec3 radius_depth_w;

/* float perspective_radius(in float r) */
/* { */
/*   // Included border */
/*   float w = 768.0 + 48.0; */
/*   float h = 768.0 + 48.0; */

/*   float fov = 1.0; */
/*   float z_near = 1.0; */

/*   float diag = sqrt (w*w + h*h); */
/*   float h_near = (h / diag) * fov * z_near * 2.0; */

/*   float r_p = r * (z_near / radius_depth_w.y) * (1.0 / (h_near) ); */
  
/* /\*   if (r_p < 10000000.0) *\/ */
/* /\*     return 0.000001; *\/ */
/* /\*   else *\/ */

/*     return r_p; */
/* } */

float perspective_radius(in float r, in vec3 n) {

  float len = length(n.xy);

  if (len == 0.0)
    n.y = 0.0;
  else
    n.y /= len;

  // angle between normal and z direction
  float angle = acos(n.y);
  float sin_angle = sin(angle);

  return r * sin_angle;
}

void main(void)
{ 
  if (radius_depth_w.x <= 0.0)
    discard;

  float depth_interval;

  depth_interval = radius_depth_w.x;

  // First buffer  : normal.x, normal.y, normal.z, radius
  // Second buffer : minimum depth, depth interval, center.x, center.y
  // Third buffer  : color
  gl_FragData[0] = vec4 (normalize(normal_vec), radius_depth_w.x / radius_depth_w.z ); 
  gl_FragData[1] = vec4 (radius_depth_w.y - depth_interval, depth_interval, 0.0, 0.0);
  gl_FragData[2] = gl_Color;
}
