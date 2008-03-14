/// GLSL CODE

/// 1st Fragment Shader

// Projects points to screen space and rotates normal
// stores output on texture
#extension GL_ARB_draw_buffers : enable
#extension GL_EXT_gpu_shader4 : enable

const float pi = 3.1416;

const float max_radius = (1.0/(816.0))*7.5;
//const float max_radius = (1.0/(1024.0))*4.5;

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

  //r *= 0.001;

  if (r > max_radius)
    r *= -1.0;
/*     discard; */
//    r = max_radius;

  // atan(y,x) returns a value in the range [-pi, pi]
  float theta = atan( normal_vec.y, normal_vec.x );
  float phi = acos( normal_vec.z );

  // [theta, phi, depth, radius]
  gl_FragColor = vec4 (theta/pi, phi/pi, z, r);
}
