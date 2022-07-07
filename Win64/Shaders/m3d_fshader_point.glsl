#version 330 core

uniform vec4 color;
uniform bool south;

void main()
{
	float l = length(gl_PointCoord.xy-vec2(0.5));
	

	{
		if(south)
		{
			if(l>0.3) discard;
			float c = smoothstep(0.0, 0.05, abs(l-0.3));
			gl_FragColor = vec4(c, 0.0, 0.0, 1.0);
		}
		else
		{
			if(l>0.45) discard;
			float c = smoothstep(0.0, 0.07, abs(l-0.4));
			gl_FragColor = vec4(1.0, c, c, 1.0-c);
		}
	}
	

}
