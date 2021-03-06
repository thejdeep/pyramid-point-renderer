#version 120

uniform sampler2D textureA;
uniform sampler2D textureB;

uniform vec4 color_ambient;
uniform vec4 color_diffuse;
uniform vec4 color_specular;
uniform float shininess;

void main (void) {

  vec4 normal = texture2D (textureA, gl_TexCoord[0].st).xyzw;
  vec4 color = texture2D (textureB, gl_TexCoord[0].st).yzwx;  

  if (normal.a != 0.0) {
	color.a = normal.a;
	normal.a = length(normal.xyz);

	color.rgb /= normal.a;
 	normal.rgb = normalize(normal.rgb);	

	if (shininess == 99.0) {
	  //color.a *= normal.b;
	  color.rgb = vec3(color.a, 1.0-color.a, 0.0);
	}
	else if (shininess == 98.0) {
	  color.rgb = normal.rgb; 
	}
	else {
	  vec3 lightDir = normalize(vec3(gl_LightSource[0].position));
     
	  color += color_ambient * (gl_LightSource[0].ambient + gl_LightModel.ambient);

	  float NdotL = max(dot(normal.xyz, lightDir.xyz), 0.0);

	  if (NdotL > 0.0) {
		color += color_diffuse * gl_LightSource[0].diffuse * NdotL;
		float NdotHV = max(dot(normal.xyz, gl_LightSource[0].halfVector.xyz), 0.0);
		color += color_specular * gl_LightSource[0].specular * pow(NdotHV, shininess);
	  }
	}
    color.a = 1.0;
  }
  else
    color = vec4(1.0, 1.0, 1.0, 0.0);
  
  gl_FragColor = color;
}
