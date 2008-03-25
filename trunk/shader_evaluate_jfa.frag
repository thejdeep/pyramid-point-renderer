/* Synthesis step */

#extension GL_ARB_draw_buffers : enable
#extension GL_EXT_gpu_shader4 : enable

#version 120

const float pi = 3.1416;
const int num_ellipses = 2;

// flag for depth test on/off
uniform bool depth_test;

uniform int step_length;

uniform float reconstruction_filter_size;
uniform float prefilter_size;

// Projected samples texture
uniform sampler2D textureA;
// Min dist texture
uniform sampler2D textureB;

// tests if a point is inside a circle.
// Circle is centered at origin, and point is
// displaced by param d.
float pointInCircle(in vec2 d, in float radius){
  float sqrt_len = d.x*d.x + d.y*d.y;

  radius *= 1.0*reconstruction_filter_size;
  radius += prefilter_size;

  float dif = sqrt_len / (radius*radius);

  //  if (dif <= reconstruction_filter_size)
  return dif;
  //else return -1.0;
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
  float a = 1.0*radius*reconstruction_filter_size;
  float b = a*normal.z;

  // include antialiasing filter (increase both axis)
  a += prefilter_size;
  b += prefilter_size;

  // inside ellipse test
  float test = ((rotated_pos.x*rotated_pos.x)/(a*a)) + ((rotated_pos.y*rotated_pos.y)/(b*b));

  //if (test <= reconstruction_filter_size)
  return test;
  //  else return -1;
}

void main (void) {
  vec4 ellipse;
  vec2 displacement;
  float dist_test;
  vec3 normal;
  float ellipse_theta, ellipse_phi;
  vec4 ellipse_coords;
  vec2 ellipse_coord[2];
  vec2 curr_closest[2] = vec2[2](vec2(0.0), vec2(0.0));

  vec2 texSizeB = vec2(textureSize2D(textureB, 0));
  vec2 min_z = vec2(-1.0);

  for (int j = -1; j <= 1; ++j) {
    for (int i = -1; i <= 1; ++i) {

      displacement = gl_TexCoord[0].st + (vec2(float(i*step_length), float(j*step_length)) / texSizeB);

      ellipse_coords = texture2D (textureB, displacement).xyzw;
      ellipse_coord[0] = ellipse_coords.xy;
      ellipse_coord[1] = ellipse_coords.zw;

      for (int k = 0; k < num_ellipses; ++k) {
	// if pixel from displacement position is a projected surfel, check if current
	// pixel is inside its radius
	if ((ellipse_coord[k] != vec2(0.0))
	    && (ellipse_coord[k] != curr_closest[0])
	    && (ellipse_coord[k] != curr_closest[1]))
	  {
	    // retrieve candidadte ellipse from displacement position
	    ellipse = texture2D (textureA, ellipse_coord[k]).xyzw;

	    // convert from spherical coordinates
	    ellipse_theta = ellipse.x * pi;
	    ellipse_phi = ellipse.y * pi;
	    normal = vec3 (cos(ellipse_theta)*sin(ellipse_phi), sin(ellipse_theta)*sin(ellipse_phi), cos(ellipse_phi));

	    dist_test = pointInEllipse((gl_TexCoord[0].st - ellipse_coord[k].xy), ellipse.w, normal);	      
	    //dist_test = pointInCircle((gl_TexCoord[0].st - ellipse_coord.xy), ellipse.w);	 	  

	    // Ellipse in range
	    if (dist_test <= 1.0) {
	      bool inserted = false;
	      // check if any slot is still free
	      for (int l = 0; l < num_ellipses; ++l) {
		if (inserted == false)
		  if (min_z[l] == -1.0) {
		    min_z[l] = ellipse.z;
		    curr_closest[l] = ellipse_coord[k];
		    // stop searching
		    inserted = true;
		  }
	      }
	      // check if it is closer than any already stored ellipse
	      for (int l = 0; l < num_ellipses; ++l) {
		if (inserted == false) {
		  // check if is closest than first slot,
		  // in this case eliminates second slot and pushes first to second
		  if ((!depth_test) || (ellipse.z <= min_z[l])) {
		    // push everyone one slot back (except last that is kicked out of line)
		    for (int m = num_ellipses-1; m > l; --m) {
		      min_z[m] = min_z[m-1];
		      curr_closest[m] = curr_closest[m-1];
		    }
		    // replace current position with ellipse
		    min_z[l] = ellipse.z;
		    curr_closest[l] = ellipse_coord[k];
		    // stop searching
		    inserted = true;
		  }
		}
	      }
	    }
	  }
      }
    }
  }

/*   if (curr_closest[0] == curr_closest[1]) */
/*     discard; */

  gl_FragColor = vec4(curr_closest[0].xy, curr_closest[1].xy);
}
