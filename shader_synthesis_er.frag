/* Synthesis step */

//#extension GL_ARB_draw_buffers : enable
#extension GL_EXT_gpu_shader4 : enable

#version 120

const float pi = 3.1416;

const float reduc_factor = 1.0;

uniform vec2 fbo_size;
uniform vec2 oo_fbo_size;
//uniform float half_pixel_size;

uniform vec2 tex_start;
uniform vec2 canvas_start;
uniform vec2 oo_tex_size;
uniform float oo_canvas_width;
uniform int level;

// flag for depth test on/off
uniform bool depth_test;

uniform ivec2 displacement;
uniform int mask_size;

uniform float reconstruction_filter_size;
uniform float prefilter_size;

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

void splatEllipse(inout vec4 buffer0, inout vec4 buffer1, 
		  in vec3 normal, in float r, in float ellipseZ, in vec2 local_displacement) {
  // if pixel from displacement position is a projected surfel, check if current
  // pixel is inside its radius

  float dist_test = pointInEllipse(local_displacement.xy, r, normal);
  //float dist_test = pointInCircle(local_displacement.xy, r);

  // Ellipse in range
  if (dist_test >= 0.0) {

    // weight is the gaussian exponential of the distance to the ellipse's center
    float weight = exp(-0.5*dist_test);
    //float weight = 1.0;

    // sum contribution to current values if pixel near current surface (elipse)
    if ((!depth_test) || (buffer0.w == 0.0) || (abs(ellipseZ - (buffer1.x / buffer0.w)) <= 1.0*r)) {
      float pixelZ = buffer1.x / buffer0.w;
      
      buffer0 += vec4(normal * weight, r * weight);
      buffer1.x += ellipseZ * weight;
      buffer1.y += weight;
    }
    // overwrite pixel if ellipse is in front or if pixel is empty, otherwise keep current pixel
    else if (ellipseZ < (buffer1.x/buffer0.w)) {
      buffer0 = vec4(normal * weight, r * weight);
      buffer1 = vec4(ellipseZ * weight, weight, 0.0, 0.0);
    }
  }
}

void main (void) {

  // retrieve actual pixel with current values
  vec4 buffer0 = texture2D (textureA, gl_TexCoord[0].st, 0).xyzw;
  vec4 buffer1 = texture2D (textureB, gl_TexCoord[0].st, 0).xyzw;
  vec4 buffer2 = texture2D (textureC, gl_TexCoord[0].st, 0).xyzw;
  vec4 ellipse0, ellipse1;
  vec2 local_displacement;


  vec2 tex_displacement = ((gl_TexCoord[0].st - canvas_start) * fbo_size) -
    ((gl_TexCoord[3].st - tex_start) * fbo_size * pow(2.0, level));

/*   vec2 tex_displacement = (gl_TexCoord[0].st - level_0_border) - */
/*     ((gl_TexCoord[3].st - tex_start) * pow(2.0, level)); */

/*   // convert to a number in pixels (whole number) */
/*   tex_displacement *= fbo_size; */

  // convert to pixel dimension
  tex_displacement *= oo_canvas_width;

  for (int j = -mask_size; j <= mask_size; ++j) {
    for (int i = -mask_size; i <= mask_size; ++i) {
      if ((i != 0) || (j != 0)) {

	//vec2 local_displacement = global_displacement.xy + (vec2(i, j) / texSizeB);
	local_displacement = vec2(i, j) * oo_fbo_size.st;
	vec2 local_pixel_displacement = (vec2(-i, -j) * pow(2.0, level)) * oo_canvas_width;

	// retrieve candidadte ellipse from displacement position
	ellipse0 = texture2D (textureA, gl_TexCoord[3].st + local_displacement.xy).xyzw;
	ellipse1 = texture2D (textureB, gl_TexCoord[3].st + local_displacement.xy).xyzw;

	if (ellipse0.w != 0.0)
	  //  splatEllipse(buffer0, buffer1, ellipse0.xyz, ellipse0.w, ellipse1.x, tex_displacement + ellipse1.zw + vec2(i, j) * half_pixel_size);
	  splatEllipse(buffer0, buffer1, ellipse0.xyz, ellipse0.w, ellipse1.x, 
		       tex_displacement + ellipse1.zw);// + local_pixel_displacement);
      }
    }
  }

  if (buffer0.w > 0.0) {
    buffer0.xyz = normalize(buffer0.xyz);
    //buffer0.w /= buffer1.y;
    buffer1.x /= buffer1.y;
  }
  else {
/*     buffer0 = vec4(0.0); */
/*     buffer1 = vec4(0.0); */
  }

  gl_FragData[0] = buffer0;
  gl_FragData[1] = buffer1;
  gl_FragData[2] = buffer2;
}
