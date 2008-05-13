/// GLSL CODE

/// 1st Fragment Shader

// Projects points to screen space and rotates normal
// stores output on texture
#extension GL_ARB_draw_buffers : enable
#extension GL_EXT_gpu_shader4 : enable

// height of near viewing plane
//uniform float h_near;

uniform vec2 oo_fbo_size;
uniform vec2 canvas_border;
uniform float min_size;

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

  vec2 texCoord = vec2( floor(gl_FragCoord.st)*oo_fbo_size );

  float radius = radius_depth_w.x / radius_depth_w.z;

  float level_zero = 1.0;
  if (radius > min_size)
    level_zero = 0.0;

  // First buffer  : normal.x, normal.y, normal.z, radius
  // Second buffer : minimum depth, sets to 1.0 if valid ellipse (correct level), center.x, center.y
  // Third buffer  : unprojected radius, - , - , color id
  gl_FragData[0] = vec4 ( normalize(normal_vec), 0.0 );
  gl_FragData[1] = vec4 ( radius_depth_w.y, level_zero, 0.0, gl_Color.w );
  gl_FragData[2] = vec4 ( radius_depth_w.x, radius, texCoord.st );
}
