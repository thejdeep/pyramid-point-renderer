/* Synthesis step */

#extension GL_ARB_draw_buffers : enable
 #extension GL_EXT_gpu_shader4 : enable

#version 120

const float pi = 3.1416;

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
  vec4 ellipse_coord;
  vec4 curr_closest = vec4(0.0);

  vec2 texSizeB = vec2(textureSize2D(textureB, 0));
  float min_dist = 100000.0;
  float min_z = -1.0;


    for (int j = -1; j <= 1; ++j) {
      for (int i = -1; i <= 1; ++i) {

	displacement = gl_TexCoord[0].st + (vec2(float(i*step_length), float(j*step_length)) / texSizeB);

	ellipse_coord = texture2D (textureB, displacement).xyzw;

	// if pixel from displacement position is a projected surfel, check if current
	// pixel is inside its radius
	if (ellipse_coord.w != 0.0) 
	  // if ((ellipse_coord.x >= 0.0) && (ellipse_coord.x < 1.0) && (ellipse_coord.y >= 0.0) && (ellipse_coord.y < 1.0))
	    {
	  
	      // retrieve candidadte ellipse from displacement position
	      ellipse = texture2D (textureA, ellipse_coord.xy).xyzw;
	  
	      // convert from spherical coordinates
	      ellipse_theta = ellipse.x * pi;
	      ellipse_phi = ellipse.y * pi;
	      normal = vec3 (cos(ellipse_theta)*sin(ellipse_phi), sin(ellipse_theta)*sin(ellipse_phi), cos(ellipse_phi));
	  
	      dist_test = pointInEllipse((gl_TexCoord[0].st - ellipse_coord.xy), ellipse.w, normal);	      
	      //dist_test = pointInCircle((gl_TexCoord[0].st - ellipse_coord.xy), ellipse.w);	 	  

	      // Ellipse in range
	      if (dist_test <= 1.0)
		if ((!depth_test) || (curr_closest.w == 0.0) || (ellipse.z - min_z <= ellipse.w))
		  //		  if (dist_test < min_dist)
		   {
		     min_dist = dist_test;
		     min_z = ellipse.z;
		     curr_closest = vec4(ellipse_coord.xy, min_dist, 1.0);
		   }
	    }
      }
    }    
  gl_FragColor = curr_closest;
}
