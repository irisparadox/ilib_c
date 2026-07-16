#define IHSTMAP_IMPLEMENTATION
#include "ihstmap.h"
#include <assert.h>
#include <stdio.h>

static void test_basic_insert_get(void) {
	ihstmap_t map;
	int a = 1, b = 2;

	assert(ihstmap_construct(&map, NULL, NULL) == IHSTMAP_OK);
	assert(ihstmap_insert(&map, &a, &b, 0) == IHSTMAP_OK);
	assert(ihstmap_get(&map, &a) == &b);

	ihstmap_destroy(&map);
	printf("test_basic_insert_get OK\n");
}

static void test_default_replace(void) {
	ihstmap_t map;
	int a = 1, b = 2, c = 3;

	assert(ihstmap_construct(&map, NULL, NULL) == IHSTMAP_OK);
	assert(ihstmap_insert(&map, &a, &b, 0) == IHSTMAP_OK);
	assert(ihstmap_insert(&map, &a, &c, 0) == IHSTMAP_OK);
	assert(ihstmap_get(&map, &a) == &c);
	assert(ihstmap_size(&map) == 1);

	ihstmap_destroy(&map);
	printf("test_default_replace OK\n");
}

static void test_istnorep(void) {
	ihstmap_t map;
	int a = 1, b = 2, c = 3;

	assert(ihstmap_construct(&map, NULL, NULL) == IHSTMAP_OK);
	assert(ihstmap_insert(&map, &a, &b, 0) == IHSTMAP_OK);
	assert(ihstmap_insert(&map, &a, &c, IHSTMAP_ALTMODE, IHSTMAP_ISTNOREP) == IHSTMAP_ERREXIST);
	assert(ihstmap_get(&map, &a) == &b);

	ihstmap_destroy(&map);
	printf("test_istnorep OK\n");
}

static void test_istretex(void) {
	ihstmap_t map;
	int a = 1, b = 2, c = 3;
	void *existing = NULL;

	assert(ihstmap_construct(&map, NULL, NULL) == IHSTMAP_OK);
	assert(ihstmap_insert(&map, &a, &b, 0) == IHSTMAP_OK);
	assert(ihstmap_insert(&map, &a, &c, IHSTMAP_ALTMODE,
	                       (ilib_uint32_t)(IHSTMAP_ISTREP | IHSTMAP_ISTRETEX), &existing) == IHSTMAP_OK);
	assert(existing == &b);
	assert(ihstmap_get(&map, &a) == &c);

	ihstmap_destroy(&map);
	printf("test_istretex OK\n");
}

static void test_prehash(void) {
	ihstmap_t map;
	int a = 1, b = 2;
	ilib_uint64_t hash;

	assert(ihstmap_construct(&map, NULL, NULL) == IHSTMAP_OK);
	hash = map.h->hash(&a, map.h->usrdata);
	assert(ihstmap_insert(&map, &a, &b, IHSTMAP_PREHASH, hash) == IHSTMAP_OK);
	assert(ihstmap_get(&map, &a) == &b);

	ihstmap_destroy(&map);
	printf("test_prehash OK\n");
}

static void test_remove(void) {
	ihstmap_t map;
	int a = 1, b = 2;

	assert(ihstmap_construct(&map, NULL, NULL) == IHSTMAP_OK);
	assert(ihstmap_insert(&map, &a, &b, 0) == IHSTMAP_OK);
	assert(ihstmap_remove(&map, &a) == IHSTMAP_OK);
	assert(ihstmap_get(&map, &a) == NULL);
	assert(ihstmap_remove(&map, &a) == IHSTMAP_ERRNTFND);

	ihstmap_destroy(&map);
	printf("test_remove OK\n");
}

static void test_tombstone_chain(void) {
	ihstmap_t map;
	int keys[3] = {1, 2, 3};
	int vals[3] = {10, 20, 30};

	assert(ihstmap_construct(&map, NULL, NULL) == IHSTMAP_OK);

	/* Insert A, remove A, insert B, confirm C (inserted after) still
	 * resolves correctly even if its probe chain crosses A's tombstone. */
	assert(ihstmap_insert(&map, &keys[0], &vals[0], 0) == IHSTMAP_OK);
	assert(ihstmap_remove(&map, &keys[0]) == IHSTMAP_OK);
	assert(ihstmap_insert(&map, &keys[1], &vals[1], 0) == IHSTMAP_OK);
	assert(ihstmap_insert(&map, &keys[2], &vals[2], 0) == IHSTMAP_OK);

	assert(ihstmap_get(&map, &keys[1]) == &vals[1]);
	assert(ihstmap_get(&map, &keys[2]) == &vals[2]);
	assert(ihstmap_get(&map, &keys[0]) == NULL);

	ihstmap_destroy(&map);
	printf("test_tombstone_chain OK\n");
}

static void test_errfull(void) {
	ihstmap_t map;
	int keys[64];
	int vals[64];
	ilib_size_t i, n;
	int hit_full;

	assert(ihstmap_construct(&map, NULL, NULL) == IHSTMAP_OK);

	n = sizeof(keys) / sizeof(keys[0]);
	hit_full = 0;

	for (i = 0; i < n; i++) {
		keys[i] = (int)i;
		vals[i] = (int)i * 10;

		int rc = ihstmap_insert(&map, &keys[i], &vals[i], IHSTMAP_ALTMODE, IHSTMAP_ISTNOREP);
		if (rc == IHSTMAP_ERRFULL) {
			hit_full = 1;
			break;
		}
		assert(rc == IHSTMAP_OK);
	}

	assert(hit_full == 1);

	ihstmap_destroy(&map);
	printf("test_errfull OK\n");
}

int main(void) {
	test_basic_insert_get();
	test_default_replace();
	test_istnorep();
	test_istretex();
	test_prehash();
	test_remove();
	test_tombstone_chain();
	test_errfull();

	printf("all tests passed\n");
	return 0;
}
