/// GLSL CODE

/// 1st Fragment Shader

// Projects points to screen space and rotates normal
// stores output on texture
#extension GL_ARB_draw_buffers : enable

// height of near viewing plane
// uniform float h_near;

uniform vec2 oo_fbo_size;
uniform float scale;

uniform float reconstruction_filter_size;
uniform float canvas_width;
uniform int mask_size;

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

  vec2 texCoord = vec2( floor(gl_FragCoord.st)*oo_fbo_size );

  float proj_minor = minor_length * scale / dist_to_eye;
  float proj_major = major_length * scale / dist_to_eye;

  float unprojected_radius = proj_major;

  int log_level = int(floor(log2( ( 2.0 * proj_major * reconstruction_filter_size * canvas_width ) / float(mask_size*2 + 1) )));

  if ((2.0*proj_major*canvas_width*reconstruction_filter_size) > float(mask_size*2 + 1))
	unprojected_radius *= -1.0;

  // First buffer : minimum depth, depth interval, center.x, center.y
  gl_FragData[0] = vec4 (4.0*unprojected_radius, radius_depth_w.y, texCoord.st);
  gl_FragData[1] = vec4 (minor_axis, proj_minor);
  gl_FragData[2] = vec4 (normal, proj_major);

}
