#include <stdio.h>

#define LINALG_IMPLEMENTATION
#include "linalg.h"

#include "graph.h"

int main(void)
{
	vec2i_t a = { 10, -5 };
	vec2i_t b = { 99, 42 };

	vmask2_t m = v2cmpgt_vv(a, b);

	vec2i_t r1 = v2select(a, b, m);
	
	printf("x: %d, y: %d\n", r1.x, r1.y);

	vec2f_t af = { 99.15, -5.0 };
	vec2f_t bf = { 99.10, 42.590};

	vmask2_t mf = vf2cmplt_vv(af, bf);

	vec2f_t r2 = vf2select(af, bf, mf);

	printf("x: %.3f, y: %.3f\n", r2.x, r2.y);
	printf("%.3f\n", vf2redmin_v(r2));
	printf("%.3f %.3f\n", vf2length_v(r2), vf2lengthsq_v(r2));

	vec2f_t norm = vf2normalize_v(r2);
	printf("x: %.3f, y: %.3f\n", norm.x, norm.y);

	vec3i_t v1 = { 1, 1, 1 };
	vec3i_t v2 = { -1, 1, 0 };

	vec3i_t cross = v3cross_vv(v1, v2);
	printf("x: %df, y: %df, z: %df\n", cross.x, cross.y, cross.z);

	return 0;
}