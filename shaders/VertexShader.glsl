#version 120

varying vec4 texCoord;
varying vec4 projCoord;
varying vec3 modelNormal;

void main() {
  texCoord = gl_MultiTexCoord0;
  projCoord = gl_TextureMatrix[1] * gl_Vertex;
  modelNormal = gl_Normal;

  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
  gl_FrontColor = gl_Color;
}
