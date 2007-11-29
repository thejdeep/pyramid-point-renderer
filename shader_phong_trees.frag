uniform sampler2D textureA;
uniform sampler2D textureC;

void main (void) {

  vec4 normal = texture2D (textureA, gl_TexCoord[0].st).xyzw;
  vec4 color = texture2D (textureC, gl_TexCoord[0].st).xyzw;

  color.a = 1.0;

  if (normal.a != 0.0) {    
    vec4 lightDir = vec4(normalize(gl_LightSource[0].position.xyz), 1.0);
   
      if (abs(color.a - 0.5) < 0.1)
    {
      normal *= -1.0;
      normal = lightDir - normal * ( dot(lightDir, normal) / dot(normal, normal) );
      /* vec3 NxL = normalize(cross(-normal.xyz, lightDir.xyz)); */
      /* normal = vec4(cross(NxL, -normal.xyz), 1.0); */
    }
    else {
      if (dot(normal.xyz, lightDir.xyz) < 0.0)
	normal *= -1.0;
    }

    normal = normalize(normal);
    
    //color += gl_FrontMaterial.ambient * gl_LightSource[0].ambient + gl_LightModel.ambient;

    float NdotL = max(dot(normal.xyz, lightDir.xyz), 0.0);
    //float NdotL = abs(dot(normal.xyz, lightDir.xyz));

    //    color += gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse * NdotL;
    color *= gl_LightSource[0].diffuse * NdotL;

/*     if (NdotL > 0.0) { */
/*      float NdotHV = max(dot(normal.xyz, gl_LightSource[0].halfVector.xyz), 0.0); */
/*      color += gl_FrontMaterial.specular * gl_LightSource[0].specular * pow(NdotHV, gl_FrontMaterial.shininess); */
/*     } */
  }
  else
    color = vec4(1.0);
  
  gl_FragColor = vec4(color.rgb, 1.0);
}
