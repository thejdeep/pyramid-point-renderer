/* Analysis step */

#extension GL_ARB_draw_buffers : enable
//#version 120

const float PI = 3.1416;

// flag for depth test on/off
uniform bool depth_test;

// one over 2 * fbo size
uniform vec2 oo_2fbo_size;

uniform vec2 fbo_size;
uniform vec2 oo_canvas_size;

uniform int mask_size;

// size of half a pixel
uniform float canvas_width;
uniform int level;

uniform bool quality_per_vertex;
uniform float quality_threshold;

uniform float reconstruction_filter_size;
uniform float prefilter_size;

uniform sampler2D textureA;
uniform sampler2D textureB;
uniform sampler2D textureC;

// tests if a point is inside an ellipse.
// Ellipse is centered at origin and point displaced by d.
// Radius is the half the ellipse's major axis.
// Minor axis is computed by normal direction.
float pointInEllipse(in vec2 d, in float minor_axis_length, in float major_axis_length, 
					 in vec3 minor_axis){	
					 
  float len = length(minor_axis.xy);

  if (len == 0.0)
    minor_axis.y = 0.0;
  else
    minor_axis.y /= len;

  // angle between minor_axis and z direction
  float angle = acos(minor_axis.y);

  if (minor_axis.x > 0.0)
    angle *= -1.0;

  // rotate point to ellipse coordinate system
  vec2 rotated_pos = vec2(d.x*cos(angle) + d.y*sin(angle), -d.x*sin(angle) + d.y*cos(angle));

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
    // Valid pixel : minor length > 0
    if (pixelB[i].w > 0.0) {

      // this ellipse has stop propagating, it lies on the finer level
      // the sign of the unprojected radius determines if it has reached the correct level (positive) or not (negative)
      if (pixelA[i].x < 0.0) {
		// test for minimum depth coordinate of valid ellipses
		//if (pixelA[i].w <= zmin) {
		//if ( ((quality_per_vertex) && (pixelC[i].w > obj_id)) || ((!quality_per_vertex) && (pixelA[i].w <= zmin)) ) 
	{
	  zmin = pixelA[i].y;
	  zmax = abs(pixelC[i].w);
	  //	  obj_id = pixelC[i].w; //only necessary if using color buffer for quality
	}
      }
      else {
	// if ellipse not in correct level ignore it during average
	pixelB[i].w = -1.0;
      }
    }
  }

  // Gather pixels values
  for (int i = 0; i < 4; ++i)
    {
      // Check if valid gather pixel or unspecified (or ellipse out of reach set above)
      if (pixelB[i].w > 0.0) 
		{
		  //if ((!quality_per_vertex) || (abs(pixelC[i].w - obj_id) <= quality_threshold) )
			{
			  // Depth test between valid in reach ellipses
			  // if ((!depth_test) || (pixelB[i].x - pixelC[i].y <= zmax))
			  if ((!depth_test) || (abs(zmin - pixelA[i].x) <= (zmax+pixelA[i].x)))
				{
				  //float w = abs(4.0 * PI * 4.0 * pixelA[i].w * pixelA[i].w * pixelA[i].z);
				  float w = 1.0;//pixelC[i].a;

				  // radius computation
				  bufferA.x = max(abs(bufferA.x), abs(pixelA[i].x));
				  //bufferA.y = max(bufferA.y, pixelA[i].y);

				  // average depth
				  bufferA.y += pixelA[i].y * w;

				  // average tex coords
				  bufferA.zw += pixelA[i].zw * w;

				  // average minor axis
				  bufferB.xyz += pixelB[i].xyz * w;
				  bufferB.w += pixelB[i].w * w;

				  // average normal
				  bufferC += pixelC[i] * w;
	      
				  valid_pixels += w;
				}
			}
		}
    }

  // average values if there are any valid ellipses
  // otherwise the pixel will be writen as unspecified  
  if (valid_pixels > 0.0)
    {
      bufferA.zw /= valid_pixels;
 
      bufferB /= valid_pixels;
      bufferB.xyz = normalize(bufferB.xyz);
      bufferC /= valid_pixels;
      bufferC.xyz = normalize(bufferC.xyz);

      // If this ellipse is on the correct pyramid level stop its propagation
      // i.e., it will not be used in the average of the next coarser level	  

      float log_level = log2( ( 2.0 * bufferC.w * reconstruction_filter_size * canvas_width ) / float(mask_size*2 + 1) );
      if (level == int( floor(log_level) ))
	bufferA.x = abs(bufferA.x);
      else
	bufferA.x = abs(bufferA.x) * -1.0;

/* 	  if ((2.0*bufferB.y*canvas_width*reconstruction_filter_size) > float(mask_size*2 + 1)) */
/* 		bufferB.x = abs(bufferB.x); */
/*       else */
/* 		bufferB.x = abs(bufferB.x) * -1.0; */
    }

  // first buffer = (n.x, n.y, n.z, radius)
  gl_FragData[0] = bufferA;
  // second buffer = (depth, max_depth, dx, dy)
  gl_FragData[1] = bufferB;
  // color value = (r, g, b, obj_id)
  gl_FragData[2] = bufferC;
}
