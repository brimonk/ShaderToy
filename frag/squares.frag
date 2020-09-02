/*
 * Brian Chrzanowski
 * Mon Feb 11, 2019 18:14
 *
 * Fun, square trip
 */

#version 300 es

precision highp float;

uniform float    uTime;
uniform int      uTick;
uniform ivec2    uResolution;
uniform ivec2    uMouse;

out vec4 FragColor;

void main()
{
	vec2 p;
	int i;
	float tmp;

	tmp = float(uTick) * 0.0002;

	p = 2.0 * gl_FragCoord.xy / 256.0;

	for (i = 1; i < 10; i++) {
		float n = float(i);

		p += floor(vec2(
			// whole lotta garbage
			0.7 / n * sin(n*p.y+tmp+0.5*n)+2.2,
			0.3 / n*sin(n*p.x+tmp+0.3*n)+4.6
			)*3.0) / 2.0 +sin((p.x - tmp) / 4.9);
	}

	FragColor.r = 0.5 * sin(p.x) + 0.5;
	FragColor.g = 0.5 * sin(p.y) + 0.5;
	FragColor.b = 0.5 * sin(p.y + p.y);
	FragColor.a = 1.0;
}
