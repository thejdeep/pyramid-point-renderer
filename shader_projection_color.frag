/// GLSL CODE

/// 1st Fragment Shader

// Projects points to screen space and rotates normal
// stores output on texture
#extension GL_ARB_draw_buffers : enable

// height of near viewing plane
//uniform float h_near;

varying vec3 normal_vec;
varying vec3 radius_depth_w;

float perspective_radius(in float r)
{
  // Included border
  float w = 768.0 + 48.0;
  float h = 768.0 + 48.0;

  float fov = 1.0;
  float z_near = 1.0;

  float diag = sqrt (w*w + h*h);
  float h_near = (h / diag) * fov * z_near * 2.0;

  float r_p = r * (z_near / radius_depth_w.y) * (1.0 / (h_near) );
  
  return r_p;
}

void main(void)
{ 
  if (radius_depth_w.x == 0.0)
    discard;

  // First buffer : normal.x, normal.y, normal.z, radius
  gl_FragData[0] = vec4 (normal_vec, (radius_depth_w.x / (radius_depth_w.z * zoom_factor)) ); 
  //gl_FragData[0] = vec4 (normal_vec, perspective_radius(radius_depth_w.x));

  //gl_FragData[0] = vec4 (normal_vec, (radius_depth_w.x /  zoom_factor) );

  float depth_interval = radius_depth_w.x;

  //float depth_interval = radius_depth_w.x * (1.0-normal_vec.z);

  // Second buffer : minimum depth, depth interval, center.x, center.y
  gl_FragData[1] = vec4 (radius_depth_w.y - depth_interval, depth_interval*2.0, 0.0, 0.0);
  gl_FragData[2] = vec4(1.0, gl_Color.gba);
}
