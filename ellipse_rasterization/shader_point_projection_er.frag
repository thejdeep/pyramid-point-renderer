/// GLSL CODE

/// 1st Fragment Shader

// Projects points to screen space and rotates normal
// stores output on texture
#version 120

#extension GL_ARB_draw_buffers : enable
#extension GL_EXT_gpu_shader4 : enable

const float pi = 3.1416;

uniform float max_radius;

varying vec3 normal_vec;
varying vec3 radius_depth_w;
//varying vec2 center;

void main(void)
{ 
  if (radius_depth_w.x <= 0.0)
    discard;

  if (normal_vec.z < 0.0)
    discard;

  float r = abs(radius_depth_w.x / radius_depth_w.z);
  float z = radius_depth_w.y;

  // Setting this to negative number so I can check later how many where over
  // the max_radius by reading the buffer and counting in CPU
  // This is for debugging and statistics purposes only
  if (r > max_radius)
    r *= -1.0;
/*     discard; */
//    r = max_radius;

  // atan(y,x) returns a value in the range [-pi, pi]
  float theta = atan( normal_vec.y, normal_vec.x );
  float phi = acos( normal_vec.z );

  //  vec2 c = center.xy / radius_depth_w.z;

  // [theta, phi, depth, radius]
  gl_FragColor = vec4 (theta/pi, phi/pi, z, r);
  //gl_FragData[0] = vec4 (theta/pi, phi/pi, z, r);
  //  gl_FragData[1] = vec4 ( c.xy, 1.0, 1.0);
}
