#ifndef FUNFUNC_H_
#define FUNFUNC_H_

#include <stdio.h>
#include <stdlib.h>

#define BUG() do { \
	fprintf(stderr, "BUG: failure at %s:%d/%s()!\n", \
		__FILE__, __LINE__, __func__); \
	abort(); \
} while (0)


#endif // FUNFUNC_H_
