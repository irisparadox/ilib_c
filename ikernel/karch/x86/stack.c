#include <deftypei.h>
#include <arch/stack.h>

void relocate_stack(void *old_stack, void *new_stack, ilib_size_t size)
{
	ilib_uintptr_t old_base;
	ilib_uintptr_t new_base;
	ilib_intptr_t delta;

	old_base = (ilib_uintptr_t)old_stack;
	new_base = (ilib_uintptr_t)new_stack;

	delta = (ilib_intptr_t)(new_base - old_base);

	ilib_uintptr_t *ptr = (ilib_uintptr_t *)new_base;

	ilib_size_t i;
	for (i = 0; i < size / sizeof(ilib_uintptr_t); ++i)
	{
		ilib_uintptr_t val = *ptr;

		if (val >= old_base &&
		    val < old_base + size)
			*ptr = (ilib_uintptr_t)((ilib_intptr_t)val + delta);
	}
}
