#version 330 core

layout (location = 0) in vec3 posAttr;

uniform mat4 transform;

out vec2 fragCoord;

void main()
{
   gl_Position = transform * vec4(posAttr, 1.0);
   fragCoord = gl_Position.xy;
}