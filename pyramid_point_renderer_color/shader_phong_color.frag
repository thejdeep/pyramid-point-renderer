#version 120

uniform sampler2D textureA;
uniform sampler2D textureC;

uniform vec4 color_ambient;
uniform vec4 color_diffuse;
uniform vec4 color_specular;
uniform float shininess;

void main (void) {

  vec4 normal = texture2D (textureA, gl_TexCoord[0].st).xyzw;
  vec4 color = texture2D (textureC, gl_TexCoord[0].st).xyzw;

  if (normal.a != 0.0) {

    normal = normalize(normal);

    vec3 lightDir = normalize(vec3(gl_LightSource[0].position));
     
	//    color += color_ambient * (gl_LightSource[0].ambient + gl_LightModel.ambient);

    color += color_ambient * (gl_LightSource[0].ambient + gl_LightModel.ambient);

	float NdotL = max(dot(normal.xyz, lightDir.xyz), 0);

    //color += diffuse[material] * gl_LightSource[0].diffuse * NdotL;

    if (NdotL > 0.0) {
      color += color_diffuse * gl_LightSource[0].diffuse * NdotL;
      float NdotHV = max(dot(normal.xyz, gl_LightSource[0].halfVector.xyz), 0.0);
      color += color_specular * gl_LightSource[0].specular * pow(NdotHV, shininess);
    }

    color.a = 1.0;
  }
  else
    color = vec4(1.0, 1.0, 1.0, 0.0);
  
  gl_FragColor = color;
}
