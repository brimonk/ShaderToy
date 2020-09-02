/*
 * Brian Chrzanowski
 * 2020-09-01 22:19:02
 *
 * Just "Red"
 */

#version 330

uniform float uTime;
uniform ivec2 uRes;
uniform ivec2 uMouse;

out vec4 oColor;

void main()
{
	oColor = vec4(1.0, 0.0, 0.0, 1.0);
}

