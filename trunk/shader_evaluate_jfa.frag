/* Synthesis step */

#extension GL_ARB_draw_buffers : enable
 #extension GL_EXT_gpu_shader4 : enable

#version 120

const float pi = 3.1416;

// flag for depth test on/off
//uniform bool depth_test;

uniform int step_length;

uniform float reconstruction_filter_size;
uniform float prefilter_size;

uniform sampler2D textureA;
uniform sampler2D textureB;

// tests if a point is inside a circle.
// Circle is centered at origin, and point is
// displaced by param d.
float pointInCircle(in vec2 d, in float radius){
  float sqrt_len = d.x*d.x + d.y*d.y;

  radius *= 1.0;
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

  //  if (len == 0.0)
  //  if ((normal.y == 0.0) || (normal.x == 0.0))
  if (normal.z == 1.0)
    //return pointInCircle(d, radius);
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
  float a = 1.0*radius;
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
  vec4 ellipse;
  vec2 displacement;
  float dist_test;
  vec4 closest_ellipse;
  vec3 normal;
  float ellipse_theta, ellipse_phi;

  vec2 texSizeB = vec2(textureSize2D(textureB, 0));

  // retrieve actual pixel with current texture coords of closest ellipse
  vec4 curr_closest = texture2D (textureB, gl_TexCoord[0].st, 0).xyzw;

  float min_dist = 100;
  if (curr_closest.xy != vec2(0.0, 0.0)) {
    // retrieve closest ellipse and compute its distance
    closest_ellipse = texture2D(textureA, curr_closest.xy).xyzw;

    ellipse_theta = closest_ellipse.x * pi;
    ellipse_phi = closest_ellipse.y * pi;
    normal = vec3 (cos(ellipse_theta)*sin(ellipse_phi), sin(ellipse_theta)*sin(ellipse_phi), cos(ellipse_phi));

    // this is the current minimum distance to be checked with new fetches
    min_dist = pointInEllipse(abs(gl_TexCoord[0].st - curr_closest.xy), closest_ellipse.w, normal);
  }

  for (int j = -1; j <= 1; ++j) {
    for (int i = -1; i <= 1; ++i) {

      displacement = gl_TexCoord[0].st + (vec2(float(i*step_length), float(j*step_length)) / texSizeB);

      vec2 ellipse_coord = texture2D (textureB, displacement).xy;

      // if pixel from displacement position is a projected surfel, check if current
      // pixel is inside its radius
      if (ellipse_coord != vec2(0.0)) {

	// retrieve candidadte ellipse from displacement position
	ellipse = texture2D (textureA, ellipse_coord).xyzw;
	ellipse.w = abs(ellipse.w); 

	// convert from spherical coordinates
	ellipse_theta = ellipse.x * pi;
	ellipse_phi = ellipse.y * pi;
	normal = vec3 (cos(ellipse_theta)*sin(ellipse_phi), sin(ellipse_theta)*sin(ellipse_phi), cos(ellipse_phi));

	dist_test = pointInEllipse(displacement.xy, ellipse.w, normal);
	//float dist_test = pointInCircle(local_displacement.xy, ellipse.w);

	// Ellipse in range
	if (dist_test < min_dist) {
	  curr_closest = vec4(displacement.xy, 1.0, 1.0);
	  min_dist = dist_test;
	}
      }
    }
  }
  gl_FragColor = curr_closest;
}
