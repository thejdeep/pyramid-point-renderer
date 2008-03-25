/// GLSL CODE

/// 1st Fragment Shader

// Projects points to screen space and rotates normal
// stores output on texture
#version 120

#extension GL_ARB_draw_buffers : enable
#extension GL_EXT_gpu_shader4 : enable

const float pi = 3.1416;

varying vec3 normal_vec;
varying vec3 radius_depth_w;

void main(void)
{ 
  if (radius_depth_w.x <= 0.0)
    discard;

  if (normal_vec.z < 0.0)
    discard;

  float r = abs(radius_depth_w.x / radius_depth_w.z);
  float z = radius_depth_w.y;

  // atan(y,x) returns a value in the range [-pi, pi]
  float theta = atan( normal_vec.y, normal_vec.x );
  float phi = acos( normal_vec.z );

  // [theta, phi, depth, radius]
  //gl_FragColor = vec4 (theta/pi, phi/pi, z, r);


  gl_FragData[0] = vec4 (theta/pi, phi/pi, z, r);
  gl_FragData[1] = vec4 (gl_FragCoord.st/1024.0, 0.0, 1.0);
  //gl_FragData[1] = vec4 (0.2, 0.2, 0.0, 0.0);
}
