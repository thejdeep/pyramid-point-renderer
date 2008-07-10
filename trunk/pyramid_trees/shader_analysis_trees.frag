/* Analysis step */

//#version 120
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
uniform sampler2D textureC;

vec2 gather_pixel_desloc[4] = vec2[4](vec2(-half_pixel_size, -half_pixel_size), 
				      vec2(half_pixel_size, -half_pixel_size), 
				      vec2(-half_pixel_size, half_pixel_size), 
				      vec2(half_pixel_size, half_pixel_size));

// tests if a point is inside a circle.
// Circle is centered at origin, and point is
// displaced by param d.
float pointInCircle(in vec2 d, in float radius){
  float sqrt_len = d.x*d.x + d.y*d.y;

  radius += prefilter_size;

  float dif = sqrt_len / (radius*radius);

  //  if (dif <= reconstruction_filter_size)
  if (dif <= 1.0)
    return dif;
  else return -1.0;
}

// tests if a point is inside an ellipse.
// Ellipse is centered at origin and point displaced by d.
// Radius is the half the ellipse's major axis.
// Minor axis is computed by normal direction.
float pointInEllipse(in vec2 d, in float radius){

  vec3 normal = vec3(0.0, 0.0, 1.0);

  // angle between normal and z direction
  float angle = 1.0;

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

  if (test <= 1.0)
    return test;
  else return -1.0;
}

// tests if a point is inside an ellipse.
// Ellipse is centered at origin and point displaced by d.
// Radius is the half the ellipse's major axis.
// Minor axis is computed by normal direction.
float pointInEllipse(in vec2 d, in float radius, in vec3 normal){
  float len = length(normal.xy);
  if (len != 0.0)
    normal.y /= len;
  else normal.y = 0.0;

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
    return 1.0;
    //    return test;
  else return -1.0;
}

void main (void) {

  vec2 tex_coord[4];
  
  vec4 bufferA = vec4(0.0, 0.0, 0.0, 0.0);
  vec4 bufferB = vec4(0.0, 0.0, 0.0, 0.0);
  vec4 bufferC = vec4(0.0, 0.0, 0.0, 0.0);

  float valid_pixels = 0.0;

  vec4 pixelA[4], pixelB[4], pixelC[4];

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
    pixelC[i] = texture2D (textureC, tex_coord[i].st).xyzw;
  }

  // Compute the front most pixel from lower level (minimum z
  // coordinate)
  float dist_test = 0.0;
  float zmin = 10000.0;
  float zmax = -10000.0;
  float obj_id = -1.0;
  for (int i = 0; i < 4; ++i) {
    if (pixelA[i].w > 0.0) {
      // test if this ellipse reaches the center of the pixel being constructed
      if (abs(pixelC[i].w - 0.5) < 0.1)
	//dist_test = pointInCircle(pixelB[i].zw + gather_pixel_desloc[i].xy, pixelA[i].w);
	dist_test = pointInEllipse(pixelB[i].zw + gather_pixel_desloc[i].xy, pixelA[i].w);
      else
	dist_test = pointInEllipse(pixelB[i].zw + gather_pixel_desloc[i].xy, pixelA[i].w, pixelA[i].xyz);
      //dist_test = pointInCircle(pixelB[i].zw + gather_pixel_desloc[i].xy, pixelA[i].w);

      if  (dist_test >= 0.0)
	{
	  // test for minimum depth coordinate of valid ellipses
	  if (pixelB[i].x <= zmin) {
	    zmin = pixelB[i].x;
	    zmax = zmin + pixelB[i].y;
	    obj_id = pixelC[i].w;
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
	
 	if (abs(pixelC[i].w - obj_id) < 0.1)
	{
	  // Depth test between valid in reach ellipses
	  if ((!depth_test) || (pixelB[i].x <= zmax)) {
	    
	    bufferA += pixelA[i];
	    
	    // Increment ellipse total path with distance from gather pixel to center
	    bufferB.zw += pixelB[i].zw + gather_pixel_desloc[i].xy;
	    
	    bufferC += pixelC[i];
	    
	    // Take maximum depth range
	    new_zmax = max(pixelB[i].x + pixelB[i].y, new_zmax);
	    
	    valid_pixels += 1.0;
	  }
	}
      }
    }

  // average values if there are any valid ellipses
  // otherwise the pixel will be writen as unspecified
  
  if (valid_pixels >= 1.0)
    {
      bufferA /= valid_pixels;
      //bufferA.xyz = normalize(bufferA.xyz);
      bufferB.x = zmin;
      bufferB.y = new_zmax - zmin;
      bufferB.zw /= valid_pixels;
      bufferC.rgb /= valid_pixels;
      bufferC.w = obj_id;
    }

  // first buffer = (n.x, n.y, n.z, radius)
  gl_FragData[0] = bufferA;
  // second buffer = (depth, max_depth, dx, dy)
  gl_FragData[1] = bufferB;
  // color value = (r, g, b, obj_id)
  gl_FragData[2] = bufferC;
}
