#version 120

uniform sampler2D textureA;
uniform sampler2D textureB;

//silver
//gold
//turquoise
//ruby
//alvaro blue

const float pi = 3.1416;
const int num_materials = 6;

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
  float a = 1.0*radius;
  float b = a*normal.z;

  // include antialiasing filter (increase both axis)
/*   a += prefilter_size; */
/*   b += prefilter_size; */

  // inside ellipse test
  float test = ((rotated_pos.x*rotated_pos.x)/(a*a)) + ((rotated_pos.y*rotated_pos.y)/(b*b));

  //if (test <= reconstruction_filter_size)
    return test;
  //  else return -1;
}

/**
 * Unompresses two coordinates in range [0, texSize[ from one 32bits float.
 * returns coordinates in range [0, 1]
 **/
vec2 uncompress(in float compressed, in vec2 tex_size) {
  float total_size = tex_size.x * tex_size.y;
  float uncompressed = compressed * total_size;
  float y = uncompressed / tex_size.y;
  float x = (y - floor(y));
  return vec2(x, floor(y) / tex_size.y);
}

void main (void) {

  vec4 closest_coord = texture2D (textureA, gl_TexCoord[0].st).xyzw;
  vec4 color = vec4(1.0);
  vec3 normal = vec3(0.0);
  vec2 ellipse_coord[2];

  ellipse_coord[0] = uncompress(closest_coord.x, vec2(1024.0, 1024.0));
  ellipse_coord[1] = uncompress(closest_coord.z, vec2(1024.0, 1024.0));

  closest_coord = vec4(ellipse_coord[0], ellipse_coord[1]);

  if (closest_coord.zw != vec2(0.0, 0.0)) {
    vec4 pixel[2];
    pixel[0] = texture2D (textureB, closest_coord.xy ).xyzw;
    pixel[1] = texture2D (textureB, closest_coord.zw ).xyzw;

    vec3 p_normal[2];
    for (int i = 0; i < 2; ++i) {
      // convert from spherical coordinates, note normal has already length = 1
      pixel[i].xy *= pi;
      p_normal[i] = vec3 (cos(pixel[i].x)*sin(pixel[i].y), sin(pixel[i].x)*sin(pixel[i].y), cos(pixel[i].y));
    }
    vec2 dist_test;
    dist_test[0] = exp(-pointInEllipse((gl_TexCoord[0].st - closest_coord.xy), pixel[0].w, p_normal[0]));
    dist_test[1] = exp(-pointInEllipse((gl_TexCoord[0].st - closest_coord.zw), pixel[1].w, p_normal[1]));
    normal = (dist_test[0] * p_normal[0] + dist_test[1] * p_normal[1]);// / (dist_test[0] + dist_test[1]);
  }
  else if (closest_coord.xy != vec2(0.0, 0.0))
    {
      vec4 pixel = texture2D (textureB, closest_coord.xy ).xyzw;
      pixel.xy *= pi;
      normal = vec3 (cos(pixel.x)*sin(pixel.y), sin(pixel.x)*sin(pixel.y), cos(pixel.y));
    }

  if (normal != vec3(0.0)) {

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
  else
    color = vec4(1.0);

  gl_FragColor = vec4(color.rgb, 1.0);
}
