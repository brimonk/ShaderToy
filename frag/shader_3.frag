/*
 * Brian Chrzanowski
 * 2020-09-01 22:19:53
 *
 * using gl_FragCoord to provide a color
 */

#version 330

uniform float uTime;
uniform ivec2 uRes;
uniform ivec2 uMouse;

out vec4 oColor;

void main()
{
	vec2 pos;

	pos = gl_FragCoord.xy / uRes;

	oColor = vec4(pos.x, pos.y, 0.0, 1.0);
}

