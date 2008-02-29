/* Synthesis step */

#extension GL_ARB_draw_buffers : enable
//#version 120

// flag for depth test on/off
uniform bool depth_test;
uniform bool elliptical_weight;

uniform float reconstruction_filter_size;
uniform float prefilter_size;

uniform ivec2 displacement;

uniform sampler2D textureA;
uniform sampler2D textureB;
uniform sampler2D textureC;

// tests if a point is inside a circle.
// Circle is centered at origin, and point is
// displaced by param d.
float pointInCircle(in vec2 d, in float radius){
  float sqrt_len = d.x*d.x + d.y*d.y;

  radius *= 2.0;
  radius += prefilter_size;

  float dif = sqrt_len / (radius*radius);

  if (dif <= reconstruction_filter_size)
    return dif;
  else return -1.0;
}

// tests if a point is inside an ellipse.
// Ellipse is centered at origin and point displaced by d.
// Radius is the half the ellipse's major axis.
// Minor axis is computed by normal direction.
// @param d Difference vector from center of ellipse to point.
// @param radius Ellipse major axis length * 0.5.
// @param normal Normal vector.
float pointInEllipse(in vec2 d, in float radius, in vec3 normal){
  float len = length(normal.xy);

  if (len == 0.0)
    normal.y = 0.0;
  else
    normal.y /= len;

  // angle between normal and z direction
  float angle = acos(normal.y);
  if (normal.x > 0.0)
    angle *= -1.0;

  // rotate point to ellipse coordinate system
  vec2 rotated_pos = vec2(d.x*cos(angle) + d.y*sin(angle),
			  -d.x*sin(angle) + d.y*cos(angle));

  // major and minor axis
  float a = 2.0*radius;
  float b = a*normal.z;

  // include antialiasing filter (increase both axis)
  a += prefilter_size;
  b += prefilter_size;

  // inside ellipse test
  float test = ((rotated_pos.x*rotated_pos.x)/(a*a)) + ((rotated_pos.y*rotated_pos.y)/(b*b));

  if (test <= reconstruction_filter_size)
    return test;
  else return -1.0;
}

void main (void) {

  vec4 buffer;
  vec4 pixelA, pixelB;
  float weight = 0.0, dist_test;

  // retrieve pixel from displacement position
  pixelA = texture2D (textureA, gl_TexCoord[0].st + displacement.xy).xyzw;

  // retrieve actual pixel with current values
  buffer = texture2D (textureC, gl_TexCoord[0].st).xyzw;

  if (pixelA.w > 0.0) {
    pixelB = texture2D (textureB, gl_TexCoord[0].st + displacement.xy).xyzw;
  
    dist_test = pointInEllipse(pixelB.zw, pixelA.w, pixelA.xyz);
    //dist_test = pointInCircle(pixelB.zw, pixelA.w);
    
    // Ellipse in range
    if (dist_test > 0.0) {
      weight = exp(-0.5*dist_test);      
      buffer += vec4(pixelA.xyz, weight);
    }
  }

  gl_FragColor = buffer;
}
