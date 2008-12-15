/// GLSL CODE

/// 1st Fragment Shader

// Projects points to screen space and rotates normal
// stores output on texture

#extension GL_ARB_draw_buffers : enable
//#extension GL_EXT_gpu_shader4 : enable

uniform float scale;

varying vec3 normal_vec;
varying vec3 radius_depth_w;

void main(void)
{ 
  if (radius_depth_w.x <= 0.0)
    discard;

  float depth_interval = radius_depth_w.x*scale;

  // First buffer  : normal.x, normal.y, normal.z, radius
  // Second buffer : minimum depth, depth interval, center.x, center.y
  // Third buffer  : color
  //  gl_FragData[0] = vec4 (normalize(normal_vec), radius_depth_w.x / radius_depth_w.z ); 

  gl_FragData[0] = vec4 (normalize(normal_vec), radius_depth_w.x*(scale/radius_depth_w.z) );
  gl_FragData[1] = vec4 (radius_depth_w.y - depth_interval, depth_interval, 0.0, 0.0);
  gl_FragData[2] = gl_Color;
}
