/* Synthesis step */
#version 120

#extension GL_ARB_draw_buffers : enable
//#version 120

// flag for depth test on/off
uniform bool depth_test;
uniform bool elliptical_weight;

uniform vec2 fbo_size;
uniform vec2 oo_fbo_size;
//uniform vec2 dest_size;
uniform float half_pixel_size;

uniform float reconstruction_filter_size;
uniform float prefilter_size;

// textures on finer level
uniform sampler2D textureA;
uniform sampler2D textureB;
uniform sampler2D textureC;

// tests if a point is inside an ellipse.
// Ellipse is centered at origin and point displaced by d.
// Radius is the half the ellipse's major axis.
// Minor axis is computed by normal direction.
float pointInEllipse(in vec2 d, in float minor_axis_length, in float major_axis_length, 
					 in vec3 minor_axis, in vec3 major_axis){	
					 
  float len = length(minor_axis.xy);

  //if (len == 0.0)
  if (minor_axis.z == 0.0)
    minor_axis.y = 0.0;
  else
    minor_axis.y /= len;

  // angle between minor_axis and z direction
  float angle = acos(minor_axis.y);
  if (minor_axis.x > 0.0)
    angle *= -1.0;
  
  float cos_angle = minor_axis.y;
  float sin_angle = sin(angle);

  // rotate point to ellipse coordinate system
  vec2 rotated_pos = vec2(d.x*cos_angle + d.y*sin_angle, -d.x*sin_angle + d.y*cos_angle);

  // major and minor axis
  float a = major_axis_length*reconstruction_filter_size;
  float b = minor_axis_length*reconstruction_filter_size;

  // include antialiasing filter
  a += prefilter_size;
  b += prefilter_size;

  // inside ellipse test
  float test = ((rotated_pos.x*rotated_pos.x)/(a*a)) + ((rotated_pos.y*rotated_pos.y)/(b*b));

  if (test <= 1.0)
    return test;
  else return -1.0;
}

void main (void) {

  vec2 tex_coord[4];

  vec4 bufferA = vec4(0.0, 0.0, 0.0, 0.0);  
  vec4 bufferB = vec4(0.0, 0.0, 0.0, 0.0);
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
    if  (bufferB.w != 0.0) {
      vec4 up_pixelA = texture2D (textureA, gl_TexCoord[3].st).xyzw;
      vec4 up_pixelB = texture2D (textureB, gl_TexCoord[3].st).xyzw;
      
      if ( (up_pixelB.w != 0.0) && (bufferA.x > up_pixelA.x + up_pixelA.y) ) {
		occluded = true;
      }
    }
  }

  // unspecified pixel (weight == 0.0) or occluded pixel
  // synthesize pixel
  if ((bufferB.w == 0.0) || occluded)
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

      // For each scatter pixel
      for (int i = 0; i < 4; ++i) {

      	// distance from synthesized pixel to center of scatter pixel
		vec2 dist_to_pixel = vec2(float(pixel_config[i*2]), float(pixel_config[i*2 + 1])) * half_pixel_size;		

		// Add distance to center of ellipse
		pixelA[i].zw += dist_to_pixel;

		// if specified scatter pixel test distance to center of ellipse
		if (pixelB[i].w > 0.0)		
	  	  dist_test = pointInEllipse(pixelA[i].zw, pixelB[i].w, pixelC[i].w, pixelB[i].xyz, pixelC[i].xyz);
		else
		  dist_test = -1.0;

		// if not specified or out of range dont use it
		if (dist_test == -1.0) {
		  weights[i] = 0.0;
		}
		else {
		  if (elliptical_weight)
			weights[i] = 1.0 - dist_test;
		  else
			weights[i] = exp(-0.5*dist_test);

		  total_weight += 1.0;

		  // depth test only for ellises in range
		  if (pixelA[i].x < zmin) {
			zmin = pixelA[i].x;
			zmax = pixelA[i].y;
		  }
		}
      }

      // If the pixel was set as occluded but there is an ellipse
      // in range that does not occlude it, do not synthesize
	  // Usually means that pixel is in a back surface near an internal silhouette
      if (occluded) {
		for (int i = 0; i < 4; ++i)
		  if ((bufferA.x <= pixelA[i].x + pixelA[i].y) && (weights[i] != 0.0))
			occluded = false;
      }

      // If the pixel was set as occluded but there are no valid
      // pixels in range to synthesize, leave as it is
      if (occluded && (total_weight == 0.0))
		occluded = false;

      // synthesize pixel
      if ((bufferB.w == 0.0) || occluded) {
		bufferA = vec4(0.0);
		bufferB = vec4(0.0);
		bufferC = vec4(0.0);
		total_weight = 0.0;
	
		for (int i = 0; i < 4; ++i) {

		  // Ellipse in range
		  if (weights[i] > 0.0)
			{
			  // Depth test between ellipses in range
			  //if ((!depth_test) || (pixelB[i].x <= zmin + zmax))
			  if ((!depth_test) || (pixelA[i].x - pixelA[i].y <= zmin + zmax)) {
				  total_weight += weights[i];
				  bufferA += weights[i] * pixelA[i];
				  bufferB += weights[i] * pixelB[i];
				  bufferC += weights[i] * pixelC[i];
				}
			}
		}

		if (total_weight > 0.0) {
		  bufferA /= total_weight;
		  bufferB /= total_weight;
		  bufferB.xyz = normalize(bufferB.xyz);
		  bufferC /= total_weight;
		  bufferC.xyz = normalize(bufferC.xyz);
		}
      }
    }
    	
  	gl_FragData[0] = bufferA;
	gl_FragData[1] = bufferB;
	gl_FragData[2] = bufferC;
}
