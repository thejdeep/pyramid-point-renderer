
uniform sampler2D tex;

uniform vec3 lightDir;
uniform vec4 color_ambient;
uniform vec4 color_diffuse;
uniform vec4 color_specular;
uniform float shininess;

void main (void) {

  vec4 normal = texture2D (tex, gl_TexCoord[0].st).xyzw;
  
  vec4 color = vec4(1.0);

  if (normal.a != 0.0) {

    normal.xyz = normalize(normal.xyz);
    vec3 halfVector = -reflect(lightDir, normal.xyz);
    
    color = color_ambient;

    float NdotL = max(dot(normal.xyz, lightDir),0.0);
    if (NdotL > 0.0) {
      color += color_diffuse * NdotL;
      float NdotHV = max(dot(normal.xyz, halfVector), 0.0);
      color += color_specular * pow(NdotHV, shininess);
    }
  }

  gl_FragColor = color;
}
