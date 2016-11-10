#version 150

in vec4 vPosition;
in vec4 vColor;
uniform vec4 bvColor;
uniform vec4 bColor;
uniform vec4 Color; 
uniform mat4 modelview;
uniform mat4 projection;
varying    vec4 color;
void main() 
{ 
  gl_Position = projection*modelview*vPosition;
  color = bColor*Color+bvColor*vColor;
}