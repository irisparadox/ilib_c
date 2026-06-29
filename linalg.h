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

VEC_DECL vec2i_t v2clamp_vx(vec2i_t v, int lo, int hi);
VEC_DECL vec3i_t v3clamp_vx(vec3i_t v, int lo, int hi);
VEC_DECL vec4i_t v4clamp_vx(vec4i_t v, int lo, int hi);

VEC_DECL vec2i_t v2clamp_vv(vec2i_t v, vec2i_t lo, vec2i_t hi);
VEC_DECL vec3i_t v3clamp_vv(vec3i_t v, vec3i_t lo, vec3i_t hi);
VEC_DECL vec4i_t v4clamp_vv(vec4i_t v, vec4i_t lo, vec4i_t hi);

VEC_DECL vec2f_t vf2clamp_vf(vec2f_t v, float lo, float hi);
VEC_DECL vec3f_t vf3clamp_vf(vec3f_t v, float lo, float hi);
VEC_DECL vec4f_t vf4clamp_vf(vec4f_t v, float lo, float hi);

VEC_DECL vec2f_t vf2clamp_vv(vec2f_t v, vec2f_t lo, vec2f_t hi);
VEC_DECL vec3f_t vf3clamp_vv(vec3f_t v, vec3f_t lo, vec3f_t hi);
VEC_DECL vec4f_t vf4clamp_vv(vec4f_t v, vec4f_t lo, vec4f_t hi);

VEC_DECL vec2i_t v2min_vx(vec2i_t v, int x);
VEC_DECL vec3i_t v3min_vx(vec3i_t v, int x);
VEC_DECL vec4i_t v4min_vx(vec4i_t v, int x);
VEC_DECL vec2i_t v2max_vx(vec2i_t v, int x);
VEC_DECL vec3i_t v3max_vx(vec3i_t v, int x);
VEC_DECL vec4i_t v4max_vx(vec4i_t v, int x);

VEC_DECL vec2f_t vf2min_vf(vec2f_t v, float x);
VEC_DECL vec3f_t vf3min_vf(vec3f_t v, float x);
VEC_DECL vec4f_t vf4min_vf(vec4f_t v, float x);
VEC_DECL vec2f_t vf2max_vf(vec2f_t v, float x);
VEC_DECL vec3f_t vf3max_vf(vec3f_t v, float x);
VEC_DECL vec4f_t vf4max_vf(vec4f_t v, float x);

VEC_DECL vec2i_t v2min_vv(vec2i_t vs1, vec2i_t vs2);
VEC_DECL vec3i_t v3min_vv(vec3i_t vs1, vec3i_t vs2);
VEC_DECL vec4i_t v4min_vv(vec4i_t vs1, vec4i_t vs2);
VEC_DECL vec2i_t v2max_vv(vec2i_t vs1, vec2i_t vs2);
VEC_DECL vec3i_t v3max_vv(vec3i_t vs1, vec3i_t vs2);
VEC_DECL vec4i_t v4max_vv(vec4i_t vs1, vec4i_t vs2);

VEC_DECL vec2f_t vf2min_vv(vec2f_t vs1, vec2f_t vs2);
VEC_DECL vec3f_t vf3min_vv(vec3f_t vs1, vec3f_t vs2);
VEC_DECL vec4f_t vf4min_vv(vec4f_t vs1, vec4f_t vs2);
VEC_DECL vec2f_t vf2max_vv(vec2f_t vs1, vec2f_t vs2);
VEC_DECL vec3f_t vf3max_vv(vec3f_t vs1, vec3f_t vs2);
VEC_DECL vec4f_t vf4max_vv(vec4f_t vs1, vec4f_t vs2);

VEC_DECL int v2redmin_v(vec2i_t v);
VEC_DECL int v3redmin_v(vec3i_t v);
VEC_DECL int v4redmin_v(vec4i_t v);

VEC_DECL int v2redmax_v(vec2i_t v);
VEC_DECL int v3redmax_v(vec3i_t v);
VEC_DECL int v4redmax_v(vec4i_t v);

VEC_DECL float vf2redmin_v(vec2f_t v);
VEC_DECL float vf3redmin_v(vec3f_t v);
VEC_DECL float vf4redmin_v(vec4f_t v);

VEC_DECL float vf2redmax_v(vec2f_t v);
VEC_DECL float vf3redmax_v(vec3f_t v);
VEC_DECL float vf4redmax_v(vec4f_t v);

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

static inline int i_linalg_clampi(int v, int lo, int hi)
{
	return v < lo ? lo : v > hi ? hi : v;
}

static inline float i_linalg_clampf(float v, float lo, float hi)
{
	return v < lo ? lo : v > hi ? hi : v;
}

static inline int i_linalg_mini(int v1, int v2)
{
	return v2 + ((v1 - v2) & ((v1 - v2) >> 31));
}

static inline int i_linalg_maxi(int v1, int v2)
{
	return v1 - ((v1 - v2) & ((v1 - v2) >> 31));
}

static inline float i_linalg_minf(float v1, float v2)
{
	return v1 < v2 ? v1 : v2;
}

static inline float i_linalg_maxf(float v1, float v2)
{
	return v1 > v2 ? v1 : v2;
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

VEC_DECL vec2i_t v2clamp_vx(vec2i_t v, int lo, int hi)
{
	return (vec2i_t){ .x = i_linalg_clampi(v.x, lo, hi),
			  .y = i_linalg_clampi(v.y, lo, hi) };
}

VEC_DECL vec3i_t v3clamp_vx(vec3i_t v, int lo, int hi)
{
	return (vec3i_t){ .x = i_linalg_clampi(v.x, lo, hi),
			  .y = i_linalg_clampi(v.y, lo, hi),
			  .z = i_linalg_clampi(v.z, lo, hi) };
}

VEC_DECL vec4i_t v4clamp_vx(vec4i_t v, int lo, int hi)
{
	return (vec4i_t){ .x = i_linalg_clampi(v.x, lo, hi),
			  .y = i_linalg_clampi(v.y, lo, hi),
			  .z = i_linalg_clampi(v.z, lo, hi),
			  .w = i_linalg_clampi(v.w, lo, hi) };
}

VEC_DECL vec2i_t v2clamp_vv(vec2i_t v, vec2i_t lo, vec2i_t hi)
{
	return (vec2i_t){ .x = i_linalg_clampi(v.x, lo.x, hi.x),
			  .y = i_linalg_clampi(v.y, lo.y, hi.y) };
}

VEC_DECL vec3i_t v3clamp_vv(vec3i_t v, vec3i_t lo, vec3i_t hi)
{
	return (vec3i_t){ .x = i_linalg_clampi(v.x, lo.x, hi.x),
			  .y = i_linalg_clampi(v.y, lo.y, hi.y),
			  .z = i_linalg_clampi(v.z, lo.z, hi.z) };
}

VEC_DECL vec4i_t v4clamp_vv(vec4i_t v, vec4i_t lo, vec4i_t hi)
{
	return (vec4i_t){ .x = i_linalg_clampi(v.x, lo.x, hi.x),
			  .y = i_linalg_clampi(v.y, lo.y, hi.y),
			  .z = i_linalg_clampi(v.z, lo.z, hi.z),
			  .w = i_linalg_clampi(v.w, lo.w, hi.w) };
}

VEC_DECL vec2f_t vf2clamp_vf(vec2f_t v, float lo, float hi)
{
	return (vec2f_t){ .x = i_linalg_clampf(v.x, lo, hi),
			  .y = i_linalg_clampf(v.y, lo, hi) };
}

VEC_DECL vec3f_t vf3clamp_vf(vec3f_t v, float lo, float hi)
{
	return (vec3f_t){ .x = i_linalg_clampf(v.x, lo, hi),
			  .y = i_linalg_clampf(v.y, lo, hi),
			  .z = i_linalg_clampf(v.z, lo, hi) };
}

VEC_DECL vec4f_t vf4clamp_vf(vec4f_t v, float lo, float hi)
{
	return (vec4f_t){ .x = i_linalg_clampf(v.x, lo, hi),
			  .y = i_linalg_clampf(v.y, lo, hi),
			  .z = i_linalg_clampf(v.z, lo, hi),
			  .w = i_linalg_clampf(v.w, lo, hi) };
}

VEC_DECL vec2f_t vf2clamp_vv(vec2f_t v, vec2f_t lo, vec2f_t hi)
{
	return (vec2f_t){ .x = i_linalg_clampf(v.x, lo.x, hi.x),
			  .y = i_linalg_clampf(v.y, lo.y, hi.y) };
}

VEC_DECL vec3f_t vf3clamp_vv(vec3f_t v, vec3f_t lo, vec3f_t hi)
{
	return (vec3f_t){ .x = i_linalg_clampf(v.x, lo.x, hi.x),
			  .y = i_linalg_clampf(v.y, lo.y, hi.y),
			  .z = i_linalg_clampf(v.z, lo.z, hi.z) };
}

VEC_DECL vec4f_t vf4clamp_vv(vec4f_t v, vec4f_t lo, vec4f_t hi)
{
	return (vec4f_t){ .x = i_linalg_clampf(v.x, lo.x, hi.x),
			  .y = i_linalg_clampf(v.y, lo.y, hi.y),
			  .z = i_linalg_clampf(v.z, lo.z, hi.z),
			  .w = i_linalg_clampf(v.w, lo.w, hi.w) };
}

VEC_DECL vec2i_t v2min_vx(vec2i_t v, int x)
{
	return (vec2i_t){ .x = i_linalg_mini(v.x, x), .y = i_linalg_mini(v.y, x) };
}

VEC_DECL vec3i_t v3min_vx(vec3i_t v, int x)
{
	return (vec3i_t){ .x = i_linalg_mini(v.x, x),
			  .y = i_linalg_mini(v.y, x),
			  .z = i_linalg_mini(v.z, x) };
}

VEC_DECL vec4i_t v4min_vx(vec4i_t v, int x)
{
	return (vec4i_t){ .x = i_linalg_mini(v.x, x),
			  .y = i_linalg_mini(v.y, x),
			  .z = i_linalg_mini(v.z, x),
			  .w = i_linalg_mini(v.w, x) };
}

VEC_DECL vec2i_t v2max_vx(vec2i_t v, int x)
{
	return (vec2i_t){ .x = i_linalg_maxi(v.x, x), .y = i_linalg_maxi(v.y, x) };
}

VEC_DECL vec3i_t v3max_vx(vec3i_t v, int x)
{
	return (vec3i_t){ .x = i_linalg_maxi(v.x, x),
			  .y = i_linalg_maxi(v.y, x),
			  .z = i_linalg_maxi(v.z, x) };
}

VEC_DECL vec4i_t v4max_vx(vec4i_t v, int x)
{
	return (vec4i_t){ .x = i_linalg_maxi(v.x, x),
			  .y = i_linalg_maxi(v.y, x),
			  .z = i_linalg_maxi(v.z, x),
			  .w = i_linalg_maxi(v.w, x) };
}

VEC_DECL vec2f_t vf2min_vf(vec2f_t v, float x)
{
	return (vec2f_t){ .x = i_linalg_minf(v.x, x), .y = i_linalg_minf(v.y, x) };
}

VEC_DECL vec3f_t vf3min_vf(vec3f_t v, float x)
{
	return (vec3f_t){ .x = i_linalg_minf(v.x, x),
			  .y = i_linalg_minf(v.y, x),
			  .z = i_linalg_minf(v.z, x) };
}

VEC_DECL vec4f_t vf4min_vf(vec4f_t v, float x)
{
	return (vec4f_t){ .x = i_linalg_minf(v.x, x),
			  .y = i_linalg_minf(v.y, x),
			  .z = i_linalg_minf(v.z, x),
			  .w = i_linalg_minf(v.w, x) };
}

VEC_DECL vec2f_t vf2max_vf(vec2f_t v, float x)
{
	return (vec2f_t){ .x = i_linalg_maxf(v.x, x), .y = i_linalg_maxf(v.y, x) };
}

VEC_DECL vec3f_t vf3max_vf(vec3f_t v, float x)
{
	return (vec3f_t){ .x = i_linalg_maxf(v.x, x),
			  .y = i_linalg_maxf(v.y, x),
			  .z = i_linalg_maxf(v.z, x) };
}

VEC_DECL vec4f_t vf4max_vf(vec4f_t v, float x)
{
	return (vec4f_t){ .x = i_linalg_maxf(v.x, x),
			  .y = i_linalg_maxf(v.y, x),
			  .z = i_linalg_maxf(v.z, x),
			  .w = i_linalg_maxf(v.w, x) };
}

VEC_DECL vec2i_t v2min_vv(vec2i_t vs1, vec2i_t vs2)
{
	return (vec2i_t){ .x = i_linalg_mini(vs1.x, vs2.x),
			  .y = i_linalg_mini(vs1.y, vs2.y) };
}

VEC_DECL vec3i_t v3min_vv(vec3i_t vs1, vec3i_t vs2)
{
	return (vec3i_t){ .x = i_linalg_mini(vs1.x, vs2.x),
			  .y = i_linalg_mini(vs1.y, vs2.y),
			  .z = i_linalg_mini(vs1.z, vs2.z) };
}

VEC_DECL vec4i_t v4min_vv(vec4i_t vs1, vec4i_t vs2)
{
	return (vec4i_t){ .x = i_linalg_mini(vs1.x, vs2.x),
			  .y = i_linalg_mini(vs1.y, vs2.y),
			  .z = i_linalg_mini(vs1.z, vs2.z),
			  .w = i_linalg_mini(vs1.w, vs2.w) };
}

VEC_DECL vec2i_t v2max_vv(vec2i_t vs1, vec2i_t vs2)
{
	return (vec2i_t){ .x = i_linalg_maxi(vs1.x, vs2.x),
			  .y = i_linalg_maxi(vs1.y, vs2.y) };
}

VEC_DECL vec3i_t v3max_vv(vec3i_t vs1, vec3i_t vs2)
{
	return (vec3i_t){ .x = i_linalg_maxi(vs1.x, vs2.x),
			  .y = i_linalg_maxi(vs1.y, vs2.y),
			  .z = i_linalg_maxi(vs1.z, vs2.z) };
}

VEC_DECL vec4i_t v4max_vv(vec4i_t vs1, vec4i_t vs2)
{
	return (vec4i_t){ .x = i_linalg_maxi(vs1.x, vs2.x),
			  .y = i_linalg_maxi(vs1.y, vs2.y),
			  .z = i_linalg_maxi(vs1.z, vs2.z),
			  .w = i_linalg_maxi(vs1.w, vs2.w) };
}

VEC_DECL vec2f_t vf2min_vv(vec2f_t vs1, vec2f_t vs2)
{
	return (vec2f_t){ .x = i_linalg_minf(vs1.x, vs2.x),
			  .y = i_linalg_minf(vs1.y, vs2.y) };
}

VEC_DECL vec3f_t vf3min_vv(vec3f_t vs1, vec3f_t vs2)
{
	return (vec3f_t){ .x = i_linalg_minf(vs1.x, vs2.x),
			  .y = i_linalg_minf(vs1.y, vs2.y),
			  .z = i_linalg_minf(vs1.z, vs2.z) };
}

VEC_DECL vec4f_t vf4min_vv(vec4f_t vs1, vec4f_t vs2)
{
	return (vec4f_t){ .x = i_linalg_minf(vs1.x, vs2.x),
			  .y = i_linalg_minf(vs1.y, vs2.y),
			  .z = i_linalg_minf(vs1.z, vs2.z),
			  .w = i_linalg_minf(vs1.w, vs2.w) };
}

VEC_DECL vec2f_t vf2max_vv(vec2f_t vs1, vec2f_t vs2)
{
	return (vec2f_t){ .x = i_linalg_maxf(vs1.x, vs2.x),
			  .y = i_linalg_maxf(vs1.y, vs2.y) };
}

VEC_DECL vec3f_t vf3max_vv(vec3f_t vs1, vec3f_t vs2)
{
	return (vec3f_t){ .x = i_linalg_maxf(vs1.x, vs2.x),
			  .y = i_linalg_maxf(vs1.y, vs2.y),
			  .z = i_linalg_maxf(vs1.z, vs2.z) };
}

VEC_DECL vec4f_t vf4max_vv(vec4f_t vs1, vec4f_t vs2)
{
	return (vec4f_t){ .x = i_linalg_maxf(vs1.x, vs2.x),
			  .y = i_linalg_maxf(vs1.y, vs2.y),
			  .z = i_linalg_maxf(vs1.z, vs2.z),
			  .w = i_linalg_maxf(vs1.w, vs2.w) };
}

VEC_DECL int v2redmin_v(vec2i_t v)
{
	return i_linalg_mini(v.x, v.y);
}

VEC_DECL int v3redmin_v(vec3i_t v)
{
	return i_linalg_mini(v.x, i_linalg_mini(v.y, v.z));
}

VEC_DECL int v4redmin_v(vec4i_t v)
{
	return i_linalg_mini(i_linalg_mini(v.x, v.y), i_linalg_mini(v.z, v.w));
}

VEC_DECL int v2redmax_v(vec2i_t v)
{
	return i_linalg_maxi(v.x, v.y);
}

VEC_DECL int v3redmax_v(vec3i_t v)
{
	return i_linalg_maxi(v.x, i_linalg_maxi(v.y, v.z));
}

VEC_DECL int v4redmax_v(vec4i_t v)
{
	return i_linalg_maxi(i_linalg_maxi(v.x, v.y), i_linalg_maxi(v.z, v.w));
}


VEC_DECL float vf2redmin_v(vec2f_t v)
{
	return i_linalg_minf(v.x, v.y);
}

VEC_DECL float vf3redmin_v(vec3f_t v)
{
	return i_linalg_minf(v.x, i_linalg_minf(v.y, v.z));
}

VEC_DECL float vf4redmin_v(vec4f_t v)
{
	return i_linalg_minf(i_linalg_minf(v.x, v.y), i_linalg_minf(v.z, v.w));
}

VEC_DECL float vf2redmax_v(vec2f_t v)
{
	return i_linalg_maxf(v.x, v.y);
}

VEC_DECL float vf3redmax_v(vec3f_t v)
{
	return i_linalg_maxf(v.x, i_linalg_maxf(v.y, v.z));
}

VEC_DECL float vf4redmax_v(vec4f_t v)
{
	return i_linalg_maxf(i_linalg_maxf(v.x, v.y), i_linalg_maxf(v.z, v.w));
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