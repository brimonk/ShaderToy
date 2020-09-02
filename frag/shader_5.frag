/*
 * Brian Chrzanowski
 * Fri Sep 06, 2019 02:23
 *
 * using the stepping function
 */

#version 330

precision mediump float;

uniform float uTime;
uniform int uTick;
uniform ivec2 uRes;
uniform ivec2 uMouse;

vec3 color_a = vec3(0.149, 0.141, 0.912);
vec3 color_b = vec3(1.000, 0.833, 0.224);

out vec4 oColor;

#define PI 3.14159265359

float plot(vec2 pos, float pct)
{
	return smoothstep(pct - 0.01, pct, pos.y) - smoothstep(pct, pct + 0.01, pos.y);
}

void main()
{
	vec2 pos;
	vec3 color, pct;
	float y;

	pos = gl_FragCoord.xy / uRes;

	pct = vec3(pos.x);

	pct.r = smoothstep(0.0, 1.0, pos.x);
	pct.g = sin(pos.x * PI);
	pct.b = pow(pos.x, 0.5);

	color = vec3(0.0);
	color.r = mix(color_a.r, color_b.r, pct.r);
	color.g = mix(color_a.g, color_b.g, pct.g);
	color.b = mix(color_a.b, color_b.b, pct.b);

	// plot transition lines for each channel
	color.r = mix(color.r, 1.0, plot(pos, pct.r));
	color.g = mix(color.g, 1.0, plot(pos, pct.g));
	color.b = mix(color.b, 1.0, plot(pos, pct.b));

	oColor = vec4(color, 1.0);
}

