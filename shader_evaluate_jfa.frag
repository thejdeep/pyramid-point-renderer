/* Synthesis step */

#extension GL_ARB_draw_buffers : enable
#extension GL_EXT_gpu_shader4 : enable

#version 120

const float pi = 3.1416;

const int num_ellipses = 8;

// flag for depth test on/off
//uniform bool depth_test;

uniform int step_length;

// Min dist texture
uniform sampler2D textureB;
uniform sampler2D textureC;

/**
 * Compresses two coordinates in range [0, texSize[ in one 32bits float.
 * expects coordinates in range [0, 1]
 **/
float compress(in float i, in float j, in vec2 tex_size) {
  float total_size = tex_size.x * tex_size.y;
  return float(j*total_size + i*tex_size.x) / total_size;
}

/**
 * Uncompresses two coordinates in range [0, texSize[ from one 32bits float.
 * returns coordinates in range [0, 1]
 **/
vec2 uncompress(in float compressed, in vec2 tex_size) {
  float uncompressed = compressed * tex_size.y;
  return vec2(fract(uncompressed), floor(uncompressed) / tex_size.y);
}

void main (void) {
  float dist = 0.0;
  vec2 displacement;
  float fetch_coords[num_ellipses];
  vec2 ellipse_coord;
  float min_coord_pos;
  float min_dist, dist_pixel;

  vec2 texSizeB = vec2(textureSize2D(textureB, 0));
  float jump = float(step_length) / texSizeB.x;

  // Current nearest pixels stored at this location
  //  float curr_coords[];
  vec4 curr_coords_0 = texture2D (textureB, gl_TexCoord[0].st).xyzw;
  vec4 curr_coords_1 = texture2D (textureC, gl_TexCoord[0].st).xyzw;

  float curr_coords[] = float[num_ellipses](curr_coords_0[0], curr_coords_0[1], curr_coords_0[2], curr_coords_0[3],
				 curr_coords_1[0], curr_coords_1[1], curr_coords_1[2], curr_coords_1[3]);


/*   float fetch_coord; */
/*   // up-left - 0 */
/*   displacement = gl_TexCoord[0].st + (vec2(-1.0,  1.0) * jump); */
/*   fetch_coords[0] = texture2D (textureC, displacement).w; */
/*   // up-center - 1 */
/*   displacement = gl_TexCoord[0].st + (vec2( 0.0,  1.0) * jump); */
/*   fetch_coords[1] = texture2D (textureC, displacement).z; */
/*   // up-right - 2 */
/*   displacement = gl_TexCoord[0].st + (vec2( 1.0,  1.0) * jump); */
/*   fetch_coords[2] = texture2D (textureC, displacement).y; */
/*   // center-left - 3 */
/*   displacement = gl_TexCoord[0].st + (vec2(-1.0,  0.0) * jump); */
/*   fetch_coords[3] = texture2D (textureC, displacement).x; */
/*   // center-right - 4 */
/*   displacement = gl_TexCoord[0].st + (vec2( 1.0,  0.0) * jump); */
/*   fetch_coords[4] = texture2D (textureB, displacement).w; */
/*   // bottom-left - 5 */
/*   displacement = gl_TexCoord[0].st + (vec2(-1.0, -1.0) * jump); */
/*   fetch_coords[5] = texture2D (textureB, displacement).z; */
/*   // bottom-center - 6 */
/*   displacement = gl_TexCoord[0].st + (vec2( 0.0, -1.0) * jump); */
/*   fetch_coords[6] = texture2D (textureB, displacement).y; */
/*   // bottom-right - 7 */
/*   displacement = gl_TexCoord[0].st + (vec2( 1.0, -1.0) * jump); */
/*   fetch_coords[7] = texture2D (textureB, displacement).x; */

/*   for (int i = 0; i < num_ellipses; ++i) { */
/*     ellipse_coord = uncompress(fetch_coords[i], texSizeB); */
    
/*     if ((curr_coords[i] == 0.0) ||  */
/* 	(length(gl_TexCoord[0].st - ellipse_coord) < length(gl_TexCoord[0].st - uncompress(curr_coords[i], texSizeB)))) */
/*       curr_coords[i] = fetch_coords[i]; */
/*   } */

  int pos = 0;
  for (int j = -1; j <= 1; ++j) {
    for (int i = -1; i <= 1; ++i) {
      //      if ((i != 0) || (j != 0))
	{

	// fetch compressed indices from position (i,j)
	displacement = gl_TexCoord[0].st + (vec2(float(i), float(j)) * jump);
	vec4 fetch_coords_0 = texture2D (textureB, displacement).xyzw;
	vec4 fetch_coords_1 = texture2D (textureC, displacement).xyzw;
	fetch_coords = float[num_ellipses](fetch_coords_0[0], fetch_coords_0[1], fetch_coords_0[2], fetch_coords_0[3],
					   fetch_coords_1[0], fetch_coords_1[1], fetch_coords_1[2], fetch_coords_1[3]);

	// uncompress current coord stored at this p
	min_coord_pos = curr_coords[pos];
	min_dist = 1000.0;
	if (min_coord_pos != 0.0)
	  min_dist = length (gl_TexCoord[0].st - uncompress(curr_coords[pos], texSizeB));

	// find the mininum distance between the current indice stored, and the four fetched indices
	for (int k = 0; k < num_ellipses; ++k) {
	  if (fetch_coords[k] != 0.0) {
	    ellipse_coord = uncompress(fetch_coords[k], texSizeB);
	    dist_pixel = length (gl_TexCoord[0].st - ellipse_coord);
	    if (dist_pixel < min_dist) {
	      min_dist = dist_pixel;
	      min_coord_pos = fetch_coords[k];
	    }
	  }
	}
	curr_coords[pos] = min_coord_pos;
	++pos;
      }
    }
  }

  gl_FragData[0] = vec4(curr_coords[0], curr_coords[1], curr_coords[2], curr_coords[3]);
  gl_FragData[1] = vec4(curr_coords[4], curr_coords[5], curr_coords[6], curr_coords[7]);
}
