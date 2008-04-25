/* Synthesis step */

//#extension GL_ARB_draw_buffers : enable
#extension GL_EXT_gpu_shader4 : enable

#version 120

const float pi = 3.1416;

const float reduc_factor = 1.0;

// flag for depth test on/off
uniform bool depth_test;

uniform ivec2 displacement;
uniform int mask_size;

uniform float reconstruction_filter_size;
uniform float prefilter_size;

uniform sampler2D textureA;
uniform sampler2D textureB;
//uniform sampler2D textureC;


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
  if (r > 0.0) {

    float dist_test = pointInEllipse(local_displacement.xy, r, normal);
    //float dist_test = pointInCircle(local_displacement.xy, ellipse.w);

    // Ellipse in range
    if (dist_test >= 0.0) {

      // weight is the gaussian exponential of the distance to the ellipse's center
      float weight = exp(-0.5*dist_test);


      // sum contribution to current values if pixel near current surface (elipse)
      if ((!depth_test) || (buffer0.w == 0.0) || (abs(ellipseZ - (buffer1.x / buffer0.w)) <= 1.0*r)) {
	float pixelZ = buffer1.x / buffer0.w;

	buffer0 = vec4(buffer0.xyz + normal*weight, buffer0.w + weight);
	buffer1 = vec4(buffer1.x + ellipseZ*weight, 0.0, 0.0, 0.0);
      }
      // overwrite pixel if ellipse is in front or if pixel is empty, otherwise keep current pixel
      else if (ellipseZ < (buffer1.x/buffer0.w)) {
	buffer0 = vec4(normal, weight);
	buffer1 = vec4(ellipseZ*weight, 0.0, 0.0, 0.0);
      }      
    }
  }
}

void main (void) {

  vec2 texSizeB = vec2(textureSize2D(textureB, 0));

  // retrieve actual pixel with current values
  //  vec4 buffer = texelFetch2D (textureB, ivec2(gl_TexCoord[0].st)*texSizeB, 0).xyzw;  
  vec4 buffer0 = texture2D (textureA, gl_TexCoord[0].st, 0).xyzw;
  vec4 buffer1 = texture2D (textureB, gl_TexCoord[0].st, 0).xyzw;

  buffer0.w = 0.0;

  //  vec2 global_displacement = (vec2(displacement) / texSizeB) * float(mask_size*2+1);
  vec2 global_displacement = vec2(0.0);//(vec2(displacement) * float(mask_size*2+1)) / texSizeB;

  for (int j = -mask_size; j <= mask_size; ++j) {
    for (int i = -mask_size; i <= mask_size; ++i) {

      // The total displacement including the global displacement for all pixels and the sub-search area (mask)
      vec2 local_displacement = global_displacement.xy + (vec2(i, j) / texSizeB);

      // retrieve candidadte ellipse from displacement position
      vec4 ellipse0 = texture2D (textureA, gl_TexCoord[0].st + local_displacement.xy).xyzw;
      vec4 ellipse1 = texture2D (textureB, gl_TexCoord[0].st + local_displacement.xy).xyzw;

      splatEllipse(buffer0, buffer1, ellipse0.xyz, ellipse0.w, ellipse1.x, local_displacement); 
    }
  }
  if (buffer0.w == 0.0)
    buffer0.w = 1.0;

  gl_FragData[0] = buffer0 / buffer0.w;
  gl_FragData[1] = buffer1 / buffer0.w;
  gl_FragData[2] = vec4(0.0, 0.0, 0.0, 0.5);
}
