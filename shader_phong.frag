
uniform sampler2D textureA;


void main (void) {

  vec4 normal = texture2D (textureA, gl_TexCoord[0].st).xyzw;
  
  vec4 color = vec4(1.0);

  if (normal.a != 0.0) {

    vec3 lightDir = normalize(vec3(gl_LightSource[0].position));

    normal.xyz = normalize(normal.xyz);
    //    vec3 halfVector = -reflect(lightDir, normal.xyz);
    
    color = gl_FrontMaterial.ambient * gl_LightSource[0].ambient + gl_LightModel.ambient;

    //    float NdotL = max(dot(normal.xyz, lightDir),0.0);
    
    float NdotL = abs(dot(normal.xyz, lightDir));

    color += gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse * NdotL;

    if (NdotL > 0.0) {
      //float NdotHV = max(dot(normal.xyz, gl_LightSource[0].halfVector.xyz),0.0);
      float NdotHV = abs(dot(normal.xyz, gl_LightSource[0].halfVector.xyz));

      color += gl_FrontMaterial.specular * gl_LightSource[0].specular 
	* pow(NdotHV, gl_FrontMaterial.shininess);
    }
  }
  gl_FragColor = color;
}
