/// GLSL CODE

/// 1st Fragment Shader

// Projects points to screen space and rotates normal
// stores output on texture
#extension GL_ARB_draw_buffers : enable
#extension GL_EXT_gpu_shader4 : enable

// height of near viewing plane
//uniform float h_near;

varying vec3 normal_vec;
varying vec3 radius_depth_w;

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

/*   if (normal_vec.z < 0.0) */
/*     discard; */

  float depth_interval;

  depth_interval = radius_depth_w.x;  

  // First buffer  : normal.x, normal.y, normal.z, radius
  // Second buffer : minimum depth, depth interval, center.x, center.y
  // Third buffer  : color
  gl_FragData[0] = vec4 (normal_vec, radius_depth_w.x / radius_depth_w.z ); 
  gl_FragData[1] = vec4 (radius_depth_w.y - depth_interval, depth_interval, 0.0, 0.0);
  gl_FragData[2] = gl_Color;
}
