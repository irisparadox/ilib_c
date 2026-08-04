#ifndef STACK_H_
#define STACK_H_

#include <deftypei.h>

extern void relocate_stack(void *old_stack, void *new_stack, ilib_size_t size);

#endif // STACK_H_
