#version 120

#extension GL_EXT_gpu_shader4 : enable

uniform sampler2D textureA;
uniform sampler2D textureB;
uniform sampler2D textureC;

//uniform int dist_type;

uniform float reconstruction_filter_size;
uniform float prefilter_size;

//silver
//gold
//turquoise
//ruby
//alvaro blue

const float pi = 3.1416;
const int num_materials = 6;
const int num_ellipses = 8;

vec4 ambient[num_materials] = vec4[num_materials](vec4(0.192250, 0.192250, 0.192250, 1.000000),
						  vec4(0.247250, 0.199500, 0.074500, 1.000000),
						  vec4(0.100000, 0.187250, 0.174500, 0.800000),
						  vec4(0.174500, 0.011750, 0.011750, 0.550000),
						  vec4(0.200000, 0.200000, 0.200000, 1.000000),
						  vec4(0.100000, 0.100000, 0.100000, 1.000000));
			
vec4 diffuse[num_materials] = vec4[num_materials] (vec4(0.507540, 0.507540, 0.507540, 1.000000),
						   vec4(0.751640, 0.606480, 0.226480, 1.000000),
						   vec4(0.396000, 0.741510, 0.691020, 0.800000),
						   vec4(0.614240, 0.041360, 0.041360, 0.550000),
						   vec4(0.920000, 0.720000, 0.210000, 1.000000),
						   vec4(0.000000, 0.500000, 1.000000, 1.000000));
			
vec4 specular[num_materials] = vec4[num_materials] (vec4(0.508273, 0.508273, 0.508273, 1.000000),
						    vec4(0.628281, 0.555802, 0.366065, 1.000000),
						    vec4(0.297254, 0.308290, 0.306678, 0.800000),			   
						    vec4(0.727811, 0.626959, 0.626959, 0.550000),
						    vec4(0.050000, 0.050000, 0.050000, 1.000000),
						    vec4(0.000000, 0.500000, 1.000000, 1.000000));

float shininess[num_materials] = float[num_materials] (51.200001,
						       51.200001,
						       12.800000,
						       76.800003,
						       1.0000000,
						       0.0000000);

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
  float a = 2.0*radius;
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

/**
 * Unompresses two coordinates in range [0, texSize[ from one 32bits float.
 * returns coordinates in range [0, 1]
 **/
vec2 uncompress(in float compressed, in vec2 tex_size) {
  float uncompressed = compressed * tex_size.y;
  return vec2(fract(uncompressed), floor(uncompressed) / tex_size.y);

}

void main (void) {

  vec4 color = vec4(1.0);

  vec4 closest_coord_0 = texture2D (textureA, gl_TexCoord[0].st).xyzw;
  vec4 closest_coord_1 = texture2D (textureB, gl_TexCoord[0].st).xyzw;

  float closest_coord[] = float[num_ellipses](closest_coord_0[0], closest_coord_0[1], closest_coord_0[2], closest_coord_0[3],
				 closest_coord_1[0], closest_coord_1[1], closest_coord_1[2], closest_coord_1[3]);

  vec3 normal = vec3(0.0);
  float total_weight = 0.0;
  vec2 ellipse_coord;

  vec2 texSizeA = vec2(textureSize2D(textureA, 0));

  // average all k-nearest ellipses
  for (int i = 0; i < num_ellipses; ++i) {
    if (closest_coord[i] != 0.0) {
      ellipse_coord = uncompress(closest_coord[i], texSizeA);

      vec4 pixel = texture2D (textureC, ellipse_coord).xyzw;
      pixel.xy *= pi;
      vec3 ellipse_normal = vec3 (cos(pixel.x)*sin(pixel.y), sin(pixel.x)*sin(pixel.y), cos(pixel.y));

      float dist = pointInEllipse((gl_TexCoord[0].st - ellipse_coord), pixel.w, ellipse_normal);

      // if ((dist_type == 0) || (dist <= 1.0)) {
      if (dist != -1.0) {
	float weight = exp(-0.5*dist);
	normal += ellipse_normal * weight;
	total_weight += weight;
      }
    }
  }

  // computes pixel color if one or more ellipses are in range
  if (total_weight > 0.0) {    
    normal /= total_weight;
    normal = normalize(normal);

    //int material = int(floor( color.a*(float(num_materials)) + 0.5 ));
    int material = 1;

    vec3 lightDir = normalize(vec3(gl_LightSource[0].position));

    color = ambient[material] * gl_LightSource[0].ambient * gl_LightModel.ambient;

    float NdotL = max(dot(normal.xyz, lightDir.xyz),0.0);

    color += diffuse[material] * gl_LightSource[0].diffuse * NdotL;

    if (NdotL > 0.0) {
      float NdotHV = max(dot(normal.xyz, gl_LightSource[0].halfVector.xyz), 0.0);
      color += specular[0] * gl_LightSource[0].specular * pow(NdotHV, shininess[0]);
    }
  }
/*   else */
/*     color = vec4(1.0); */

  gl_FragColor = vec4(color.rgb, 1.0);
}
