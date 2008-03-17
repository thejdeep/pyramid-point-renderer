/* Synthesis step */

#extension GL_ARB_draw_buffers : enable
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

  vec2 texSizeB = vec2(textureSize2D(textureB, 0));

  // retrieve actual pixel with current values
  //  vec4 buffer = texelFetch2D (textureB, ivec2(gl_TexCoord[0].st)*texSizeB, 0).xyzw;  
  vec4 buffer = texture2D (textureB, gl_TexCoord[0].st, 0).xyzw;
  //  vec2 global_displacement = (vec2(displacement) / texSizeB) * float(mask_size*2+1);
  vec2 global_displacement = (vec2(displacement) * float(mask_size*2+1)) / texSizeB;

  for (int j = -mask_size; j <= mask_size; ++j) {
    for (int i = -mask_size; i <= mask_size; ++i) {

      // The total displacement including the global displacement for all pixels and the sub-search area (mask)
      vec2 local_displacement = global_displacement.xy + (vec2(i, j) / texSizeB);

      // retrieve candidadte ellipse from displacement position
      vec4 ellipse = texture2D (textureA, gl_TexCoord[0].st + local_displacement.xy).xyzw;
      ellipse.w = abs(ellipse.w);
  
      // if pixel from displacement position is a projected surfel, check if current
      // pixel is inside its radius
      if (ellipse.w > 0.0) {

	// convert from spherical coordinates
	float ellipse_theta = ellipse.x * pi;
	float ellipse_phi = ellipse.y * pi;
	vec3 normal = vec3 (cos(ellipse_theta)*sin(ellipse_phi), sin(ellipse_theta)*sin(ellipse_phi), cos(ellipse_phi));

	float dist_test = pointInEllipse(local_displacement.xy, ellipse.w, normal);
	//float dist_test = pointInCircle(local_displacement.xy, ellipse.w);

	// Ellipse in range
	if (dist_test >= 0.0) {

	  // weight is the gaussian exponential of the distance to the ellipse's center
	  float weight = exp(-0.5*dist_test)*reduc_factor;
	  float pixelZ = buffer.z / buffer.w;

	  // sum contribution to current values if pixel near current surface (elipse)
	  if ((!depth_test) || (buffer.w == 0.0) || (abs(ellipse.z - pixelZ) <= 2.0*ellipse.w)) {
	    buffer.xy *= pi;
	    vec3 curr_normal = vec3 (cos(buffer.x)*sin(buffer.y), sin(buffer.x)*sin(buffer.y), cos(buffer.y));

	    curr_normal *= buffer.w;
	    curr_normal += normal * weight;

	    curr_normal = normalize( curr_normal );
	    float theta = atan( curr_normal.y, curr_normal.x );
	    float phi = acos( curr_normal.z );

	    buffer = vec4(theta/pi, phi/pi, buffer.z + ellipse.z*weight, buffer.w + weight);
	  }
	  // overwrite pixel if ellipse is in front or if pixel is empty, otherwise keep current pixel
	  else if (ellipse.z < (buffer.z/buffer.w)) {
	    buffer = vec4(ellipse.xy, ellipse.z*weight, weight);
	  }      
	}
      }
    }
  }
  gl_FragColor = buffer;
}
