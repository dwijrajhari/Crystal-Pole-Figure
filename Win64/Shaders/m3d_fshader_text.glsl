#version 330 core

uniform sampler2D text;
uniform vec3 textColor;

in vec2 TexCoords;


void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    gl_FragColor = vec4(textColor, 1.0) * sampled;
}  