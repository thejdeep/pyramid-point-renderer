/* Analysis step */

//# version 120

#extension GL_ARB_draw_buffers : enable


// flag for depth test on/off
uniform bool depth_test;

// one over 2 * fbo size
uniform vec2 oo_2fbo_size;

// size of half a pixel
uniform float half_pixel_size;

uniform float reconstruction_filter_size;
uniform float prefilter_size;

uniform sampler2D textureA;
uniform sampler2D textureB;

vec2 gather_pixel_desloc[4] = vec2[4](vec2(-half_pixel_size, -half_pixel_size), 
				      vec2(half_pixel_size, -half_pixel_size), 
				      vec2(-half_pixel_size, half_pixel_size), 
				      vec2(half_pixel_size, half_pixel_size));


// tests if a point is inside a circle.
// Circle is centered at origin, and point is
// displaced by param d.
float pointInCircle(in vec2 d, in float radius) {
  float sqrt_len = d.x*d.x + d.y*d.y;

  radius += prefilter_size;

  float dif = sqrt_len / (radius*radius);

  if (dif <= reconstruction_filter_size)
    return dif;
  else return -1.0;
}


/**
 * Compute the intersection of an ellipse (axis oriented) and a line
 * segment.
 * Obtained from http://www.kevlindev.com/
 * @param p Center of ellipse
 * @param rx Major axis of ellipse
 * @param ry Minor axis of ellipse
 * @param a1 Point of line segment
 * @param a2 Point of line segment
 * @return 0 if no intersection, 1 if segment intersects ellipse, 2 if
 * segment is contained inside the ellipse
 **/
int intersectEllipseLine (in vec2 p, in float rx, in float ry, in vec2 a1, in vec2 a2) {
  vec2 origin = a1;
  vec2 dir = a2 - a1;
  vec2 center = p;
  vec2 diff = origin - center;
  vec2 mDir = vec2(dir.x/(rx*rx), dir.y/(ry*ry));
  vec2 mDiff = vec2(diff.x/(rx*rx), diff.y/(ry*ry));

  float a = dot(dir, mDir);
  float b = dot(dir, mDiff);
  float c = dot(diff, mDiff) - 1.0;
  float d = b*b - a*c;

  if (d < 0.0)
    return 0;

  if ( d > 0.0 ) {
    float root = sqrt(d);
    float t_a = (-b - root) / a;
    float t_b = (-b + root) / a;
    if ( ((t_a < 0.0) || (1.0 < t_a)) && ((t_b < 0.0) || (1.0 < t_b)) ) {
      if ( ((t_a < 0.0) && (t_b < 0.0)) || ((t_a > 1.0) && (t_b > 1.0)) )
	return 0;
      else
	return 2;
    }
    else
      return 1;
  } 
  else {
    float t = -b/a;
    if ( (0.0 <= t) && (t <= 1.0) )
      return 1;
    else
      return 0;
  }
}

/**
 * Intersection between a pixel's box and an ellipse.
 * @param pixel Given pixel.
 * @param point Center of pixel.
 * @param unit Half the size of a pixel, orthogonal distance from
 * @param center to boundaries of pixel.
 * @return 1 if ellipse intersects or is inside pixel, 0 otherwise.
 **/
float intersectEllipsePixel (in vec2 d, in float radius, in vec3 normal, in float unit){

  vec2 center = {0.0, 0.0};

  // rotate point to ellipse's coordinate system
  vec2 desloc_point = d;
			
  // check if ellipse center is inside box
  if (((center[0] >= desloc_point[0] - unit) && (center[0] <= desloc_point[0] + unit)) &&
      ((center[1] >= desloc_point[1] - unit) && (center[1] <= desloc_point[1] + unit)))
    return 1.0;

  // projected normal length
  float len = length(normal.xy);
  normal.y /= len;

  // ellipse rotation angle
  float angle = acos(normal.y);
  if (normal.x > 0.0)
    angle *= -1.0;

  // major and minor axis
  float a = 2.0*radius*reconstruction_filter_size;
  float b = a*normal.z;

  // include antialiasing filter
  a += prefilter_size;
  b += prefilter_size;
    

  // rotated pixel box to match ellipse coordinate system
  // box order = | 2  3 |
  //             | 0  1 |

  float cos_angle = cos(angle);
  float sin_angle = sin(angle);

  vec2 rot_box[4] = {
    vec2((desloc_point[0] - unit)*cos_angle + (desloc_point[1] - unit)*sin_angle,
	 -(desloc_point[0] - unit)*sin_angle + (desloc_point[1] - unit)*cos_angle),

    vec2((desloc_point[0] + unit)*cos_angle + (desloc_point[1] - unit)*sin_angle,
	 -(desloc_point[0] + unit)*sin_angle + (desloc_point[1] - unit)*cos_angle),

    vec2((desloc_point[0] - unit)*cos_angle + (desloc_point[1] + unit)*sin_angle,
	 -(desloc_point[0] - unit)*sin_angle + (desloc_point[1] + unit)*cos_angle),

    vec2((desloc_point[0] + unit)*cos_angle + (desloc_point[1] + unit)*sin_angle,
	 -(desloc_point[0] + unit)*sin_angle + (desloc_point[1] + unit)*cos_angle)};

  // ellipse intersects the pixels box
  if (((intersectEllipseLine(center, a, b, rot_box[0], rot_box[1]) > 0) ||
       (intersectEllipseLine(center, a, b, rot_box[2], rot_box[0]) > 0) ||
       (intersectEllipseLine(center, a, b, rot_box[3], rot_box[1]) > 0) ||
       (intersectEllipseLine(center, a, b, rot_box[3], rot_box[2]) > 0)))
    return 1.0;

  // ellipse totally outside pixel without intersection
  return -1.0;
}


// tests if a point is inside an ellipse.
// Ellipse is centered at origin and point displaced by d.
// Radius is the half the ellipse's major axis.
// Minor axis is computed by normal direction.
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

  // include antialiasing filter
  a += prefilter_size;
  b += prefilter_size;

  // inside ellipse test
  float test = ((rotated_pos.x*rotated_pos.x)/(a*a)) + ((rotated_pos.y*rotated_pos.y)/(b*b));

  if (test <= reconstruction_filter_size)
    return test;
  else return -1.0;
}

void main (void) {

  vec2 tex_coord[4];
  
  vec4 bufferA = vec4(0.0, 0.0, 0.0, 0.0);
  vec4 bufferB = vec4(0.0, 0.0, 0.0, 0.0);

  float valid_pixels = 0.0;

  vec4 pixelA[4], pixelB[4];

  //up-right
  tex_coord[0].st = gl_TexCoord[0].st + oo_2fbo_size.st;
  //up-left
  tex_coord[1].s = gl_TexCoord[0].s - oo_2fbo_size.s;
  tex_coord[1].t = gl_TexCoord[0].t + oo_2fbo_size.t;
  //down-right
  tex_coord[2].s = gl_TexCoord[0].s + oo_2fbo_size.s;
  tex_coord[2].t = gl_TexCoord[0].t - oo_2fbo_size.t;
  //down-left
  tex_coord[3].st = gl_TexCoord[0].st - oo_2fbo_size.st;

  // Gather pixels values
  for (int i = 0; i < 4; ++i) {
    pixelA[i] = texture2D (textureA, tex_coord[i].st).xyzw;
    pixelB[i] = texture2D (textureB, tex_coord[i].st).xyzw;
  }

  // Compute the front most pixel from lower level (minimum z
  // coordinate)
  float dist_test = 0.0;
  float zmin = 10000.0;
  float zmax = -10000.0;
  for (int i = 0; i < 4; ++i) {
    if (pixelA[i].w > 0.0) {
      // test if this ellipse reaches the center of the pixel being constructed
      dist_test = pointInEllipse(pixelB[i].zw + gather_pixel_desloc[i].xy, pixelA[i].w, pixelA[i].xyz);
      //dist_test = pointInCircle(pixelB[i].zw + gather_pixel_desloc[i].xy, pixelA[i].w);
      //dist_test = intersectEllipsePixel (pixelB[i].zw + gather_pixel_desloc[i].xy, pixelA[i].w, pixelA[i].xyz, half_pixel_size*2.0);

      if  (dist_test >= 0.0)
	{
	  // test for minimum depth coordinate of valid ellipses
	  if (pixelB[i].x <= zmin) {
	    zmin = pixelB[i].x;
	    zmax = zmin + pixelB[i].y;
	  }	  
	}
      else {
	// if the ellipse does not reach the center ignore it in the averaging
	pixelA[i].w = -1.0;
      }
    }
  }

  float new_zmax = zmax;

  // Gather pixels values
  for (int i = 0; i < 4; ++i)
    {
      // Check if valid gather pixel or unspecified (or ellipse out of reach set above)
      if (pixelA[i].w > 0.0) {
	
	// Depth test between valid in reach ellipses
	//	if ((!depth_test) || (pixelB[i].x <= zmax)) 
	  if ((!depth_test) || (pixelB[i].x - pixelB[i].y <= zmax)) 
	  {

	  bufferA += pixelA[i];

	  // Increment ellipse total path with distance from gather pixel to center
	  bufferB.zw += pixelB[i].zw + gather_pixel_desloc[i].xy;
	  
	  // Take maximum depth range
	  new_zmax = max(pixelB[i].x + pixelB[i].y, new_zmax);
	  
	  valid_pixels += 1.0;
	}
      }
    }

  // average values if there are any valid ellipses
  // otherwise the pixel will be writen as unspecified
  if (valid_pixels >= 1.0) {
    bufferA /= valid_pixels;
    bufferA.xyz = normalize(bufferA.xyz);
    bufferB.x = zmin;
    bufferB.y = new_zmax - zmin;
    bufferB.zw /= valid_pixels;

  }

  // first buffer = (n.x, n.y, n.z, radius)
  gl_FragData[0] = bufferA;
  // second buffer = (depth, max_depth, dx, dy)
  gl_FragData[1] = bufferB;
}
