/// GLSL CODE

/// 1st Fragment Shader

// Projects points to screen space and rotates normal
// stores output on texture
// #extension GL_ARB_draw_buffers : enable

varying float id;

void main(void)
{ 
  if (id == -1.0)
    discard;

  gl_FragColor = vec4(id, 1.0, 0.0, 1.0);
}
