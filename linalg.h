/* SPDX-License-Identifier: MIT */
#define LINALG_IMPLEMENTATION
/*
 * Linear algebra types and operations for float and int vectors.
 * Covers vec2, vec3, and vec4 for both types.
 *
 * In exactly one translation unit define LINALG_IMPLEMENTATION
 * before including to emit function definitions. All other
 * translation units include normally and get declarations.
 *
 * Define VEC_DECL before including to override default linkage.
 */

#ifndef LINALG_H
#define LINALG_H

#ifndef VEC_DECL
#define VEC_DECL static inline
#endif

#define EPS 1e-6f

typedef struct { int x, y; }       vec2i_t;
typedef struct { int x, y, z; }    vec3i_t;
typedef struct { int x, y, z, w; } vec4i_t;

typedef struct { float x, y; }       vec2f_t;
typedef struct { float x, y, z; }    vec3f_t;
typedef struct { float x, y, z, w; } vec4f_t;

// -------------------------------------------------

/* * Arithmetic functions.
 * 	op = add, sub, rsub, scale
 *	S  = 2, 3, 4
 *	vSop_vv  => res = vs1 (op) vs2
 *	vSop_vx  => res = vs1 (op) x
 *
 *	vfSop_vv => res = vs1 (op) vs2
 *	vfSop_vf => res = vs1 (op) f
 */

VEC_DECL vec2i_t v2add_vv(vec2i_t vs1, vec2i_t vs2);
VEC_DECL vec3i_t v3add_vv(vec3i_t vs1, vec3i_t vs2);
VEC_DECL vec4i_t v4add_vv(vec4i_t vs1, vec4i_t vs2);

VEC_DECL vec2i_t v2add_vx(vec2i_t vs1, int x);
VEC_DECL vec3i_t v3add_vx(vec3i_t vs1, int x);
VEC_DECL vec4i_t v4add_vx(vec4i_t vs1, int x);

VEC_DECL vec2f_t vf2add_vv(vec2f_t vs1, vec2f_t vs2);
VEC_DECL vec3f_t vf3add_vv(vec3f_t vs1, vec3f_t vs2);
VEC_DECL vec4f_t vf4add_vv(vec4f_t vs1, vec4f_t vs2);

VEC_DECL vec2f_t vf2add_vf(vec2f_t vs1, float f);
VEC_DECL vec3f_t vf3add_vf(vec3f_t vs1, float f);
VEC_DECL vec4f_t vf4add_vf(vec4f_t vs1, float f);

VEC_DECL vec2i_t v2sub_vv(vec2i_t vs1, vec2i_t vs2);
VEC_DECL vec3i_t v3sub_vv(vec3i_t vs1, vec3i_t vs2);
VEC_DECL vec4i_t v4sub_vv(vec4i_t vs1, vec4i_t vs2);

VEC_DECL vec2i_t v2sub_vx(vec2i_t vs1, int x);
VEC_DECL vec3i_t v3sub_vx(vec3i_t vs1, int x);
VEC_DECL vec4i_t v4sub_vx(vec4i_t vs1, int x);

VEC_DECL vec2i_t v2rsub_vx(vec2i_t vs1, int x);
VEC_DECL vec3i_t v3rsub_vx(vec3i_t vs1, int x);
VEC_DECL vec4i_t v4rsub_vx(vec4i_t vs1, int x);

VEC_DECL vec2f_t vf2sub_vv(vec2f_t vs1, vec2f_t vs2);
VEC_DECL vec3f_t vf3sub_vv(vec3f_t vs1, vec3f_t vs2);
VEC_DECL vec4f_t vf4sub_vv(vec4f_t vs1, vec4f_t vs2);

VEC_DECL vec2f_t vf2sub_vf(vec2f_t vs1, float f);
VEC_DECL vec3f_t vf3sub_vf(vec3f_t vs1, float f);
VEC_DECL vec4f_t vf4sub_vf(vec4f_t vs1, float f);

VEC_DECL vec2f_t vf2rsub_vf(vec2f_t vs1, float f);
VEC_DECL vec3f_t vf3rsub_vf(vec3f_t vs1, float f);
VEC_DECL vec4f_t vf4rsub_vf(vec4f_t vs1, float f);

VEC_DECL vec2i_t v2scale_vx(vec2i_t vs1, int x);
VEC_DECL vec3i_t v3scale_vx(vec3i_t vs1, int x);
VEC_DECL vec4i_t v4scale_vx(vec4i_t vs1, int x);

VEC_DECL vec2f_t vf2scale_vf(vec2f_t vs1, float f);
VEC_DECL vec3f_t vf3scale_vf(vec3f_t vs1, float f);
VEC_DECL vec4f_t vf4scale_vf(vec4f_t vs1, float f);

// -------------------------------------------------

VEC_DECL int v2dot_vv(vec2i_t vs1, vec2i_t vs2);
VEC_DECL int v3dot_vv(vec3i_t vs1, vec3i_t vs2);
VEC_DECL int v4dot_vv(vec4i_t vs1, vec4i_t vs2);

VEC_DECL float vf2dot_vv(vec2f_t vs1, vec2f_t vs2);
VEC_DECL float vf3dot_vv(vec3f_t vs1, vec3f_t vs2);
VEC_DECL float vf4dot_vv(vec4f_t vs1, vec4f_t vs2);

VEC_DECL int   v2cross_vv(vec2i_t vs1, vec2i_t vs2);
VEC_DECL float vf2cross_vv(vec2f_t vs1, vec2f_t vs2);
VEC_DECL vec3i_t v3cross_vv(vec3i_t vs1, vec3i_t vs2);
VEC_DECL vec3f_t vf3cross_vv(vec3f_t vs1, vec3f_t vs2);

// Length
VEC_DECL float vf2length_v(vec2f_t v);
VEC_DECL float vf3length_v(vec3f_t v);
VEC_DECL float vf4length_v(vec4f_t v);

// Squared length
VEC_DECL float vf2lengthsq_v(vec2f_t v);
VEC_DECL float vf3lengthsq_v(vec3f_t v);
VEC_DECL float vf4lengthsq_v(vec4f_t v);

// Normalize
VEC_DECL vec2f_t vf2normalize_v(vec2f_t v);
VEC_DECL vec3f_t vf3normalize_v(vec3f_t v);
VEC_DECL vec4f_t vf4normalize_v(vec4f_t v);

#endif // LINALG_H

#ifdef LINALG_IMPLEMENTATION
#ifndef I_LINALG_IMPLEMENTATION
#define I_LINALG_IMPLEMENTATION

// INTERNAL

static inline float i_linalg_rsqrtf(float x)
{
	float x2 = x * 0.5f;
	float y = x;
	int i;
	__builtin_memcpy(&i, &y, sizeof(i));
	i = 0x5f3759df - (i >> 1);
	__builtin_memcpy(&y, &i, sizeof(y));
	y = y * (1.5f - (x2 * y * y));
	y = y * (1.5f - (x2 * y * y));
	return y;
}

// -------------------------------------------------
// ARITHMETIC IMPLEMENTATIONS
// -------------------------------------------------

VEC_DECL vec2i_t v2add_vv(vec2i_t vs1, vec2i_t vs2)
{
	return (vec2i_t){ .x = vs1.x + vs2.x, .y = vs1.y + vs2.y };
}

VEC_DECL vec3i_t v3add_vv(vec3i_t vs1, vec3i_t vs2)
{
	return (vec3i_t){ .x = vs1.x + vs2.x,
			  .y = vs1.y + vs2.y,
			  .z = vs1.z + vs2.z };
}

VEC_DECL vec4i_t v4add_vv(vec4i_t vs1, vec4i_t vs2)
{
	return (vec4i_t){ .x = vs1.x + vs2.x,
			  .y = vs1.y + vs2.y,
			  .z = vs1.z + vs2.z,
			  .w = vs1.w + vs2.w };
}

VEC_DECL vec2i_t v2add_vx(vec2i_t vs1, int x)
{
	return (vec2i_t){ .x = vs1.x + x, .y = vs1.y + x };
}

VEC_DECL vec3i_t v3add_vx(vec3i_t vs1, int x)
{
	return (vec3i_t){ .x = vs1.x + x,
			  .y = vs1.y + x,
			  .z = vs1.z + x };
}

VEC_DECL vec4i_t v4add_vx(vec4i_t vs1, int x)
{
	return (vec4i_t){ .x = vs1.x + x,
			  .y = vs1.y + x,
			  .z = vs1.z + x,
			  .w = vs1.w + x };
}

VEC_DECL vec2f_t vf2add_vv(vec2f_t vs1, vec2f_t vs2)
{
	return (vec2f_t){ .x = vs1.x + vs2.x, .y = vs1.y + vs2.y };
}

VEC_DECL vec3f_t vf3add_vv(vec3f_t vs1, vec3f_t vs2)
{
	return (vec3f_t){ .x = vs1.x + vs2.x,
			  .y = vs1.y + vs2.y,
			  .z = vs1.z + vs2.z };
}

VEC_DECL vec4f_t vf4add_vv(vec4f_t vs1, vec4f_t vs2)
{
	return (vec4f_t){ .x = vs1.x + vs2.x,
			  .y = vs1.y + vs2.y,
			  .z = vs1.z + vs2.z,
			  .w = vs1.w + vs2.w };
}

VEC_DECL vec2f_t vf2add_vf(vec2f_t vs1, float f)
{
	return (vec2f_t){ .x = vs1.x + f, .y = vs1.y + f };
}

VEC_DECL vec3f_t vf3add_vf(vec3f_t vs1, float f)
{
	return (vec3f_t){ .x = vs1.x + f,
			  .y = vs1.y + f,
			  .z = vs1.z + f };
}

VEC_DECL vec4f_t vf4add_vf(vec4f_t vs1, float f)
{
	return (vec4f_t){ .x = vs1.x + f,
			  .y = vs1.y + f,
			  .z = vs1.z + f,
			  .w = vs1.w + f };
}

VEC_DECL vec2i_t v2sub_vv(vec2i_t vs1, vec2i_t vs2)
{
	return (vec2i_t){ .x = vs1.x - vs2.x, .y = vs1.y - vs2.y };
}

VEC_DECL vec3i_t v3sub_vv(vec3i_t vs1, vec3i_t vs2)
{
	return (vec3i_t){ .x = vs1.x - vs2.x,
			  .y = vs1.y - vs2.y,
			  .z = vs1.z - vs2.z };
}

VEC_DECL vec4i_t v4sub_vv(vec4i_t vs1, vec4i_t vs2)
{
	return (vec4i_t){ .x = vs1.x - vs2.x,
			  .y = vs1.y - vs2.y,
			  .z = vs1.z - vs2.z,
			  .w = vs1.w - vs2.w };
}

VEC_DECL vec2i_t v2sub_vx(vec2i_t vs1, int x)
{
	return (vec2i_t){ .x = vs1.x - x, .y = vs1.y - x };
}

VEC_DECL vec3i_t v3sub_vx(vec3i_t vs1, int x)
{
	return (vec3i_t){ .x = vs1.x - x,
			  .y = vs1.y - x,
			  .z = vs1.z - x };
}

VEC_DECL vec4i_t v4sub_vx(vec4i_t vs1, int x)
{
	return (vec4i_t){ .x = vs1.x - x,
			  .y = vs1.y - x,
			  .z = vs1.z - x,
			  .w = vs1.w - x };
}

VEC_DECL vec2i_t v2rsub_vx(vec2i_t vs1, int x)
{
	return (vec2i_t){ .x = x - vs1.x, .y = x - vs1.y };
}

VEC_DECL vec3i_t v3rsub_vx(vec3i_t vs1, int x)
{
	return (vec3i_t){ .x = x - vs1.x,
			  .y = x - vs1.y,
			  .z = x - vs1.z };
}

VEC_DECL vec4i_t v4rsub_vx(vec4i_t vs1, int x)
{
	return (vec4i_t){ .x = x - vs1.x,
			  .y = x - vs1.y,
			  .z = x - vs1.z,
			  .w = x - vs1.w };
}

VEC_DECL vec2f_t vf2sub_vv(vec2f_t vs1, vec2f_t vs2)
{
	return (vec2f_t){ .x = vs1.x - vs2.x, .y = vs1.y - vs2.y };
}

VEC_DECL vec3f_t vf3sub_vv(vec3f_t vs1, vec3f_t vs2)
{
	return (vec3f_t){ .x = vs1.x - vs2.x,
			  .y = vs1.y - vs2.y,
			  .z = vs1.z - vs2.z };
}

VEC_DECL vec4f_t vf4sub_vv(vec4f_t vs1, vec4f_t vs2)
{
	return (vec4f_t){ .x = vs1.x - vs2.x,
			  .y = vs1.y - vs2.y,
			  .z = vs1.z - vs2.z,
			  .w = vs1.w - vs2.w };
}

VEC_DECL vec2f_t vf2sub_vf(vec2f_t vs1, float f)
{
	return (vec2f_t){ .x = vs1.x - f, .y = vs1.y - f };
}

VEC_DECL vec3f_t vf3sub_vf(vec3f_t vs1, float f)
{
	return (vec3f_t){ .x = vs1.x - f,
			  .y = vs1.y - f,
			  .z = vs1.z - f };
}

VEC_DECL vec4f_t vf4sub_vf(vec4f_t vs1, float f)
{
	return (vec4f_t){ .x = vs1.x - f,
			  .y = vs1.y - f,
			  .z = vs1.z - f,
			  .w = vs1.w - f };
}

VEC_DECL vec2f_t vf2rsub_vf(vec2f_t vs1, float f)
{
	return (vec2f_t){ .x = f - vs1.x, .y = f - vs1.y };
}

VEC_DECL vec3f_t vf3rsub_vf(vec3f_t vs1, float f)
{
	return (vec3f_t){ .x = f - vs1.x,
			  .y = f - vs1.y,
			  .z = f - vs1.z };
}

VEC_DECL vec4f_t vf4rsub_vf(vec4f_t vs1, float f)
{
	return (vec4f_t){ .x = f - vs1.x,
			  .y = f - vs1.y,
			  .z = f - vs1.z,
			  .w = f - vs1.w };
}

VEC_DECL vec2i_t v2scale_vx(vec2i_t vs1, int x)
{
	return (vec2i_t){ .x = vs1.x * x, .y = vs1.y * x };
}

VEC_DECL vec3i_t v3scale_vx(vec3i_t vs1, int x)
{
	return (vec3i_t){ .x = vs1.x * x,
			  .y = vs1.y * x,
			  .z = vs1.z * x };
}
VEC_DECL vec4i_t v4scale_vx(vec4i_t vs1, int x)
{
	return (vec4i_t){ .x = vs1.x * x,
			  .y = vs1.y * x,
			  .z = vs1.z * x,
			  .w = vs1.w * x };
}

VEC_DECL vec2f_t vf2scale_vf(vec2f_t vs1, float f)
{
	return (vec2f_t){ .x = vs1.x * f, .y = vs1.y * f };
}

VEC_DECL vec3f_t vf3scale_vf(vec3f_t vs1, float f)
{
	return (vec3f_t){ .x = vs1.x * f,
			  .y = vs1.y * f,
			  .z = vs1.z * f };
}

VEC_DECL vec4f_t vf4scale_vf(vec4f_t vs1, float f)
{
	return (vec4f_t){ .x = vs1.x * f,
			  .y = vs1.y * f,
			  .z = vs1.z * f,
			  .w = vs1.w * f };
}

// -------------------------------------------------
// VECTORIAL IMPLEMENTATION
// -------------------------------------------------

VEC_DECL int v2dot_vv(vec2i_t vs1, vec2i_t vs2)
{
	return vs1.x * vs2.x + vs1.y * vs2.y;
}

VEC_DECL int v3dot_vv(vec3i_t vs1, vec3i_t vs2)
{
	return vs1.x * vs2.x + vs1.y * vs2.y + vs1.z * vs2.z;
}

VEC_DECL int v4dot_vv(vec4i_t vs1, vec4i_t vs2)
{
	return vs1.x * vs2.x + vs1.y * vs2.y + vs1.z * vs2.z + vs1.w * vs2.w;
}

VEC_DECL float vf2dot_vv(vec2f_t vs1, vec2f_t vs2)
{
	return vs1.x * vs2.x + vs1.y * vs2.y;
}

VEC_DECL float vf3dot_vv(vec3f_t vs1, vec3f_t vs2)
{
	return vs1.x * vs2.x + vs1.y * vs2.y + vs1.z * vs2.z;
}

VEC_DECL float vf4dot_vv(vec4f_t vs1, vec4f_t vs2)
{
	return vs1.x * vs2.x + vs1.y * vs2.y + vs1.z * vs2.z + vs1.w * vs2.w;
}

VEC_DECL int v2cross_vv(vec2i_t vs1, vec2i_t vs2)
{
	return vs1.x * vs2.y - vs1.y * vs2.x;
}

VEC_DECL float vf2cross_vv(vec2f_t vs1, vec2f_t vs2)
{
	return vs1.x * vs2.y - vs1.y * vs2.x;
}

VEC_DECL vec3i_t v3cross_vv(vec3i_t vs1, vec3i_t vs2)
{
	return (vec3i_t){ .x = vs1.y * vs2.z - vs1.z * vs2.y,
			  .y = vs1.z * vs2.x - vs1.x * vs2.z,
			  .z = vs1.x * vs2.y - vs1.y * vs2.x };
}

VEC_DECL vec3f_t vf3cross_vv(vec3f_t vs1, vec3f_t vs2)
{
	return (vec3f_t){ .x = vs1.y * vs2.z - vs1.z * vs2.y,
			  .y = vs1.z * vs2.x - vs1.x * vs2.z,
			  .z = vs1.x * vs2.y - vs1.y * vs2.x };
}

VEC_DECL float vf2length_v(vec2f_t v)
{
	float len2 = vf2dot_vv(v, v);

	if (len2 < EPS) return 0.0f;

	return len2 * i_linalg_rsqrtf(len2);
}

VEC_DECL float vf3length_v(vec3f_t v)
{
	float len2 = vf3dot_vv(v, v);

	if (len2 < EPS) return 0.0f;

	return len2 * i_linalg_rsqrtf(len2);
}

VEC_DECL float vf4length_v(vec4f_t v)
{
	float len2 = vf4dot_vv(v, v);

	if (len2 < EPS) return 0.0f;

	return len2 * i_linalg_rsqrtf(len2);
}

VEC_DECL float vf2lengthsq_v(vec2f_t v)
{
	return vf2dot_vv(v, v);
}

VEC_DECL float vf3lengthsq_v(vec3f_t v)
{
	return vf3dot_vv(v, v);
}

VEC_DECL float vf4lengthsq_v(vec4f_t v)
{
	return vf4dot_vv(v, v);
}

VEC_DECL vec2f_t vf2normalize_v(vec2f_t v)
{
	float len2 = vf2lengthsq_v(v);
	float rlen2 = i_linalg_rsqrtf(len2);
	return (vec2f_t){ .x = v.x * rlen2, .y = v.y * rlen2 };
}

VEC_DECL vec3f_t vf3normalize_v(vec3f_t v)
{
	float len2 = vf3lengthsq_v(v);
	float rlen2 = i_linalg_rsqrtf(len2);
	return (vec3f_t){ .x = v.x * rlen2,
			  .y = v.y * rlen2,
			  .z = v.z * rlen2 };
}

VEC_DECL vec4f_t vf4normalize_v(vec4f_t v)
{
	float len2 = vf4lengthsq_v(v);
	float rlen2 = i_linalg_rsqrtf(len2);
	return (vec4f_t){ .x = v.x * rlen2,
			  .y = v.y * rlen2,
			  .z = v.z * rlen2,
			  .w = v.w * rlen2 };
}

#endif // I_LINALG_IMPLEMENTATION
#endif // LINALG_IMPLEMENTATION

/*
 * MIT License
 *
 * Copyright (c) 2026 Gabriel Guillermo Saavedra Moreira
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */