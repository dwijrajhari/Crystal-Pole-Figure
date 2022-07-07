#version 330 core

uniform vec4 wireColor;
uniform int azimuthalGridDensity;
uniform int polarGridDensity;


in vec2 fragCoord;

float line(vec2 p, vec2 p0, vec2 p1)
{
    vec2 a = p-p0; 
    vec2 b = p1-p0; 
    
    vec2 proj = clamp((dot(a,b)/dot(b,b)),0.0,1.0)*b; 
    vec2 rejc = a-proj; 
    
    return 2*smoothstep(0.0f,0.0002f,dot(rejc, rejc));
}

void main()
{
	int n = azimuthalGridDensity, m = polarGridDensity;
	float step = 1.0f/n;
	float theta = 3.1416/m;
	float c = 1.0f;
	for(int i=0; i<=n; i++)
	{
		c = min(c, abs(length(fragCoord) - step*i + 0.1));
	}
	for(int i = 0; i<m; i++)
	{
		vec2 p0 = vec2(0.9*cos(i*theta), 0.9*sin(i*theta));
		c= min(c, line(fragCoord, p0, -p0));
	}
	gl_FragColor = vec4(vec3(1.0 - smoothstep(0.0f, 0.006f, c)), 1.0);
}


//-------------------------------------
// #version 330 core
// 
// uniform vec4 wireColor;
// uniform int azimuthalGridDensity;
// uniform int polarGridDensity;
// 
// 
// in vec2 fragCoord;
// 
// float line(vec2 p, vec2 p0, vec2 p1)
// {
//     vec2 a = p-p0; 
//     vec2 b = p1-p0; 
//     
//     vec2 proj = clamp((dot(a,b)/dot(b,b)),0.0,1.0)*b; 
//     vec2 rejc = a-proj; 
//     
//     return 2*smoothstep(0.0f,0.0002f,dot(rejc, rejc));
// }
// 
// void main()
// {
// 	int n = azimuthalGridDensity, m = polarGridDensity;
// 	float step = 1.0f/n;
// 	float theta = 3.1416/m;
// 	float c = 1.0f;
// 	for(int i=0; i<=n; i++)
// 	{
// 		c = min(c, abs(length(fragCoord) - step*i + 0.1));
// 	}
// 	for(int i = 0; i<m; i++)
// 	{
// 		vec2 p0 = vec2(0.9*cos(i*theta), 0.9*sin(i*theta));
// 		c= min(c, line(fragCoord, p0, -p0));
// 	}
// 	gl_FragColor = vec4(vec3(smoothstep(0.0f, 0.006f, c)), 1.0);
// }