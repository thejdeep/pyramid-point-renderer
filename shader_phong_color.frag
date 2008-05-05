uniform sampler2D textureA;
uniform sampler2D textureC;

//uniform int color_per_lod;

//silver
//gold
//turquoise
//ruby

const int num_materials = 6;

vec4 ambient[num_materials] = vec4[num_materials](vec4(0.192250, 0.192250, 0.192250, 0.500000),
						  vec4(0.247250, 0.199500, 0.074500, 1.000000),
						  vec4(0.100000, 0.187250, 0.174500, 0.800000),
						  vec4(0.174500, 0.011750, 0.011750, 0.550000),
						  vec4(0.200000, 0.200000, 0.200000, 1.000000),
						  vec4(0.100000, 0.100000, 0.100000, 1.000000));
			
vec4 diffuse[num_materials] = vec4[num_materials] (vec4(0.507540, 0.507540, 0.507540, 0.500000),
						   vec4(0.751640, 0.606480, 0.226480, 1.000000),
						   vec4(0.396000, 0.741510, 0.691020, 0.800000),
						   vec4(0.614240, 0.041360, 0.041360, 0.550000),
						   vec4(0.920000, 0.720000, 0.210000, 1.000000),
						   vec4(0.000000, 0.500000, 1.000000, 1.000000));
			
vec4 specular[num_materials] = vec4[num_materials] (vec4(0.508273, 0.508273, 0.508273, 0.500000),
						    vec4(0.628281, 0.555802, 0.366065, 1.000000),
						    vec4(0.297254, 0.308290, 0.306678, 0.800000),			   
						    vec4(0.727811, 0.626959, 0.626959, 0.550000),
						    vec4(0.050000, 0.050000, 0.050000, 1.000000),
						    vec4(0.000000, 0.500000, 1.000000, 1.000000));

float shininess[num_materials] = float[num_materials] (125.0000,
						       51.200001,
						       12.800000,
						       76.800003,
						       1.0000000,
						       0.0000000);

void main (void) {

  vec4 normal = texture2D (textureA, gl_TexCoord[0].st).xyzw;
  vec4 color = texture2D (textureC, gl_TexCoord[0].st).xyzw;

  if (normal.a != 0.0) {
    int material = int(floor( color.a*(float(num_materials)) + 0.5 ));

    normal = normalize(normal);

    if (material == 5) {
      color = normal;
      color.xyz += 1.0;
      color.xyz *= 0.5;
    }
    else 
    {
      vec3 lightDir = normalize(vec3(gl_LightSource[0].position));
     
      color = ambient[material] * (gl_LightSource[0].ambient + gl_LightModel.ambient);

      float NdotL = max(dot(normal.xyz, lightDir.xyz), 0.0);

      //      color += diffuse[material] * gl_LightSource[0].diffuse * NdotL;

      if (NdotL > 0.0) {
	color += diffuse[material] * gl_LightSource[0].diffuse * NdotL;
	float NdotHV = max(dot(normal.xyz, gl_LightSource[0].halfVector.xyz), 0.0);
	color += specular[material] * gl_LightSource[0].specular * pow(NdotHV, shininess[material]);
      }
    }
  }
  else
    color = vec4(1.0);
  
  gl_FragColor = vec4(color.rgb, 1.0);
}
