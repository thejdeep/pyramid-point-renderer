
uniform sampler2D tex;

void main (void) {

  vec4 normal = texture2D (tex, gl_TexCoord[0].st).xyzw;
  
  vec4 color = vec4(0.5);

  if (normal.a != 0.0) {

    color.xyz = normal.xyz;
  }

  gl_FragColor = color;
}
