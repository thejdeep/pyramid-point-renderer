/// GLSL CODE

/// 1st Fragment Shader

// Projects points to screen space and rotates normal
// stores output on texture
#extension GL_ARB_draw_buffers : enable

// height of near viewing plane
// uniform float h_near;

uniform float scale;

varying vec3 minor_axis;
varying vec3 normal;
varying float minor_length;
varying float major_length;
varying float depth;
varying float dist_to_eye;

void main(void)
{
  if (minor_length <= 0.0)
    discard;
  if (major_length <= 0.0)
    discard;

  float proj_minor = minor_length * scale / dist_to_eye;
  float proj_major = major_length * scale / dist_to_eye;
  float depth_interval = proj_major;

  // First buffer : minimum depth, depth interval, center.x, center.y
  gl_FragData[0] = vec4 (depth, 2.0*depth_interval, 0.0, 0.0);
  gl_FragData[1] = vec4 (minor_axis, proj_minor);
  gl_FragData[2] = vec4 (normal, proj_major);
}
