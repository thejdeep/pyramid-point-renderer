/* Synthesis step */
#version 120

#extension GL_ARB_draw_buffers : enable

// flag for depth test on/off
uniform bool depth_test;
uniform bool elliptical_weight;

uniform vec2 fbo_size;
uniform vec2 oo_fbo_size;
//uniform vec2 dest_size;
uniform float half_pixel_size;

uniform float reconstruction_filter_size;
uniform float prefilter_size;

//uniform int level;

// textures on finer level
uniform sampler2D textureA;
uniform sampler2D textureB;
uniform sampler2D textureC;

// tests if a point is inside a circle.
// Circle is centered at origin, and point is
// displaced by param d.
float pointInCircle(in vec2 d, in float radius){
  float sqrt_len = d.x*d.x + d.y*d.y;

  radius *= 1.0;
  radius += prefilter_size;

  float dif = sqrt_len / (radius*radius);

  if (dif <= reconstruction_filter_size)
    //if (dif <= 1.0)
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

  vec3 normal_vec = normal;
  float len = length(normal.xy);
  
  if (len == 0.0)
    normal.y = 0.0;
  else
    normal.y /= len;

  // angle between normal and z direction
  float angle = acos(normal.y);
  if (normal.x > 0.0)
    angle *= -1.0;

  float cos_angle = normal.y;
  float sin_angle = sin(angle);

  // rotate point to ellipse coordinate system
  vec2 rotated_pos = vec2(d.x*cos_angle + d.y*sin_angle,
						  -d.x*sin_angle + d.y*cos_angle);

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
  return -1.0;
}

void main (void) {

  vec2 tex_coord[4];

  // first buffer = (n.x, n.y, n.z, weight)
  vec4 bufferA = vec4(0.0, 0.0, 0.0, 0.0);
  // second buffer = (depth, dx, dy, radius)
  vec4 bufferB = vec4(0.0, 0.0, 0.0, 0.0);
  // third buffer = color
  vec4 bufferC = vec4(0.0, 0.0, 0.0, 0.0);
  vec4 pixelA[4], pixelB[4], pixelC[4];

  // retrieve pixel from analysis pyramid
  bufferA = texture2D (textureA, gl_TexCoord[0].st).xyzw;
  bufferB = texture2D (textureB, gl_TexCoord[0].st).xyzw;
  bufferC = texture2D (textureC, gl_TexCoord[0].st).xyzw;

  // Occlusion test - if this pixel is far behind this position
  // one level up in the pyramid it is synthesized since it is
  // occluded
  // (test the pixel z value with the depth range of the above pixel)
  bool occluded = false;

  if (depth_test) {
    if  (bufferA.w != 0.0) {
      vec4 up_pixelA = texture2D (textureA, gl_TexCoord[3].st).xyzw;
      vec4 up_pixelB = texture2D (textureB, gl_TexCoord[3].st).xyzw;
      if ( (up_pixelA.w != 0.0) && (bufferB.x  - bufferB.y > up_pixelB.x + up_pixelB.y) )
		occluded = true;
    }
  }

/*   if (level < 0) */
/* 	occluded = true; */

  // unspecified pixel (weight == 0.0) or occluded pixel
  // synthesize pixel
  if ((bufferA.w == 0.0) || occluded)
    {
      // coordinates for pixel on up-right position of coarser level
      tex_coord[0].st = fbo_size * gl_TexCoord[3].st;
      vec2 fractional = fract(tex_coord[0].st);
      tex_coord[0].st += fractional;
  
      //up-right
      tex_coord[0].st *= oo_fbo_size;
      //up-left
      tex_coord[1].st = tex_coord[0].st;
      tex_coord[1].s -= oo_fbo_size.s;
      //down-right
      tex_coord[2].st = tex_coord[0].st;
      tex_coord[2].t -= oo_fbo_size.t;
      //down-left
      tex_coord[3].st = tex_coord[0].st - oo_fbo_size.st;

      // weights
      vec4 weights;
      vec2 tmp = vec2(1.0, 1.0) - fractional;
    
      // down-left
      weights.x = fractional.x * fractional.y;
      // down-right
      weights.y = tmp.x * fractional.y;
      // up-left
      weights.z = fractional.x * tmp.y;
      // up-right
      weights.w = tmp.x * tmp.y;

      // lookup four pixels on coarser level (color attachment 0)
      pixelA[0] = texture2D(textureA, tex_coord[0]); // up-right
      pixelA[1] = texture2D(textureA, tex_coord[1]); // up-left
      pixelA[2] = texture2D(textureA, tex_coord[2]); // down-right
      pixelA[3] = texture2D(textureA, tex_coord[3]); // down-left

      // lookup four pixels on coarser level (color attachment 2)
      pixelB[0] = texture2D(textureB, tex_coord[0]); // up-right
      pixelB[1] = texture2D(textureB, tex_coord[1]); // up-left
	  pixelB[2] = texture2D(textureB, tex_coord[2]); // down-right
      pixelB[3] = texture2D(textureB, tex_coord[3]); // down-left

      // lookup four pixels on coarser level (color attachment 4)
      pixelC[0] = texture2D(textureC, tex_coord[0]); // up-right
      pixelC[1] = texture2D(textureC, tex_coord[1]); // up-left
      pixelC[2] = texture2D(textureC, tex_coord[2]); // down-right
      pixelC[3] = texture2D(textureC, tex_coord[3]); // down-left

      // for each of the four pixel configurations determine the 
      // displacement vector to each scatter pixel
      int pixel_config[8];
      // closer pixel up-right
      if ((weights.x > weights.y) && (weights.x > weights.z) && (weights.x > weights.w))
		{
		  pixel_config[0] = -3;
		  pixel_config[1] = -3;
		  pixel_config[2] =  1;
		  pixel_config[3] = -3;
		  pixel_config[4] = -3;
		  pixel_config[5] =  1;
		  pixel_config[6] =  1;
		  pixel_config[7] =  1;
		}
	  // closer pixel up-left
      else if ((weights.y > weights.z) && (weights.y > weights.w)) {
		pixel_config[0] = -1;
		pixel_config[1] = -3;
		pixel_config[2] =  3;
		pixel_config[3] = -3;
		pixel_config[4] = -1;
		pixel_config[5] =  1;
		pixel_config[6] =  3;
		pixel_config[7] =  1;
      }
      // closer pixel down-right
      else if (weights.z > weights.w) {
		pixel_config[0] = -3;
		pixel_config[1] = -1;
		pixel_config[2] =  1;
		pixel_config[3] = -1;
		pixel_config[4] = -3;
		pixel_config[5] =  3;
		pixel_config[6] =  1;
		pixel_config[7] =  3;
      }
      // closer pixel on down-left
      else {  
		pixel_config[0] = -1;
		pixel_config[1] = -1;
		pixel_config[2] =  3;
		pixel_config[3] = -1;
		pixel_config[4] = -1;
		pixel_config[5] =  3;
		pixel_config[6] =  3;
		pixel_config[7] =  3;
      }

      // invert order or invert everything above
      weights = weights.wzyx;

      float total_weight = 0.0;
      float dist_test;
      float zmin = 10000.0;
      float zmax = -zmin;
      float obj_id = -1.0;
    
      // For each scatter pixel
      for (int i = 0; i < 4; ++i) {

      	// distance from synthesized pixel to center of scatter pixel
		vec2 dist_to_pixel = vec2(float(pixel_config[i*2]), float(pixel_config[i*2 + 1])) * half_pixel_size;

		// Add distance to center of ellipse
		pixelB[i].zw += dist_to_pixel;

		// if specified scatter pixel test distance to center of ellipse
		
		if ((pixelA[i].w > 0.0)) {
		  //		  if ((pixelC[i].w > 0.1) || (!elliptical_weight))
			{
			  dist_test = pointInEllipse(pixelB[i].zw, pixelA[i].w, pixelA[i].xyz);
			  //dist_test = intersectEllipsePixel (pixelB[i].zw, pixelA[i].w, pixelA[i].xyz, half_pixel_size);
			  //dist_test = pointInCircle(pixelB[i].zw, pixelA[i].w);
			}
		}
		else
		  dist_test = -1.0;

		// if not specified or out of range dont use it
		if ((pixelA[i].w == 0.0) || (dist_test == -1.0)) {
		  weights[i] = 0.0;
		}
		else {
/* 		  if (elliptical_weight) */
/* 			weights[i] = 1.0 - dist_test; */
/* 		  else */
			//			weights[i] = 1.0;
		  //		  weights[i] = exp(-0.5*(1.0-dist_test));
		  weights[i] = 1.0 - dist_test;
		  total_weight += 1.0;

		  // depth test only for ellises in range
		  //		  
		  if (elliptical_weight){
			if (obj_id < pixelC[i].w) {

			  zmin = pixelB[i].x;
			  zmax = pixelB[i].y;
			  obj_id = pixelC[i].w;
			}
		  }
		  else{
			if (pixelB[i].x < zmin) {
			  zmin = pixelB[i].x;
			  zmax = pixelB[i].y;
			  obj_id = pixelC[i].w;
			}
		  }
		}
      }

      // If the pixel was set as occluded but there is an ellipse
      // in range that does not occlude it, do not synthesize
/* 	  if (occluded) { */
/* 	  	for (int i = 0; i < 4; ++i) */
/* 	  	  if ((bufferB.x <= pixelB[i].x + pixelB[i].y) && (weights[i] != 0.0)) */
/* 	  	    occluded = false; */
/* 	  } */

      // If the pixel was set as occluded but there are no valid
      // pixels in range to synthesize, leave as it is
      if (occluded && (total_weight == 0.0))
		occluded = false;

      // synthesize pixel
      if ((bufferA.w == 0.0) || occluded) {
		bufferA = vec4(0.0);
		bufferB = vec4(0.0);
		bufferC = vec4(0.0);
		total_weight = 0.0;
	
		for (int i = 0; i < 4; ++i) {

		  // Ellipse in range
		  if (weights[i] > 0.0)
			{
			  //if ((!elliptical_weight) || ( abs(pixelC[i].w - obj_id) < 0.2 ) ) 
			  {
				// Depth test between ellipses in range
				if ((!depth_test) || (pixelB[i].x - pixelB[i].y <= zmin + zmax)) {		  
				  total_weight += weights[i];
				  bufferA += weights[i] * pixelA[i];
				  bufferB += weights[i] * pixelB[i];
				  bufferC += weights[i] * pixelC[i];
				}
			  }
			}
		}

		if (total_weight > 0.0) 
		  {
			bufferA /= total_weight;
			bufferA.xyz = normalize(bufferA.xyz);
			bufferB /= total_weight;
			bufferC.rgb /= total_weight;
			bufferC.w = obj_id;
		  }
      }
    }
 

  // first buffer = (n.x, n.y, n.z, radius)
  gl_FragData[0] = bufferA;
  // second buffer = (depth min, depth range, dx, dy)
  gl_FragData[1] = bufferB;
  // third buffer = color
  gl_FragData[2] = bufferC;
}
