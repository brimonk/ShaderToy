/*
 * Brian Chrzanowski
 * Fri Sep 06, 2019 02:23
 *
 * using the stepping function
 */

#version 330

uniform float uTime;
uniform ivec2 uRes;
uniform ivec2 uMouse;

out vec4 oColor;

#define PI 3.14159265

float plot(vec2 pos, float pct)
{
	return smoothstep(pct - 0.02, pct, pos.y) - smoothstep(pct, pct + 0.02, pos.y);
}

void main()
{
	vec2 pos;
	vec3 color;
	float y, pct;

	pos = gl_FragCoord.xy / uRes;

	// THIS IS OUR FUNCTION
	// y = pos.x;
	// y = pow(pos.x, 5.0);
	// y = sin(pos.x * PI);

	// plot our line
	pct = plot(pos, y);
	color = vec3(y);
	// color = (1.0 - pct) * color + pct * vec3(0.0, 1.0, 0.0);
	color = pct * vec3(0.0, 1.0, 0.0);

	oColor = vec4(color, 1.0);
}


