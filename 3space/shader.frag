R"(
#version 330
//uniform vec4 color;
in vec4 fColor;
layout (location = 0) out vec4 frag_out0;
void main()
{
  //GLfloat[2] range = glGet(GL_DEPTH_RANGE);
  frag_out0 = fColor;
}
)"